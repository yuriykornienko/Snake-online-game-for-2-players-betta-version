#pragma once
#include "net_common.h"
#include "nrt_tsqueue.h"
#include "net_message.h"
#include "net_connection.h"

namespace olc {

	namespace net {
		template <typename T>
		class client_interface
		{
		public:
			client_interface()
			{}

			virtual ~client_interface()
			{
				// Если клиент уничтожен, всегда пытайтесь отключиться от сервера
				Disconnect();
			}

		public:
			// Подключитесь к серверу, указав имя хоста/ip-адрес и порт
			bool Connect(const std::string& host, const uint16_t port)
			{
				try
				{
					// Преобразовать имя хоста/ip-адрес в реальный физический адрес
					asio::ip::tcp::resolver resolver(m_context);
					asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

					// Создать соединение
					m_connection = std::make_unique<connection<T>>(connection<T>::owner::client, m_context, asio::ip::tcp::socket(m_context), m_qMessagesIn);

					// Укажите объекту подключения, что он должен подключиться к серверу
					m_connection->ConnectToServer(endpoints);

					// Запустить контекстный поток
					thrContext = std::thread([this]() { m_context.run(); });
				}
				catch (std::exception& e)
				{
					std::cerr << "Client Exception: " << e.what() << "\n";
					return false;
				}
				return true;
			}

			// Отключиться от сервера
			void Disconnect()
			{
				// Если соединение существует, и тогда оно подключено...
				if (IsConnected())
				{
					// ...изящно отключиться от сервера
					m_connection->Disconnect();
				}

				// В любом случае, мы также закончили с контекстом asio...				
				m_context.stop();
				// ...и его поток
				if (thrContext.joinable())
					thrContext.join();

				// Dудалить объект подключения
				m_connection.release();
			}

			// Проверьте, действительно ли клиент подключен к серверу
			bool IsConnected()
			{
				if (m_connection)
					return m_connection->IsConnected();
				else
					return false;
			}

		public:
			// Отправить сообщение на сервер
			void Send(const message<T>& msg)
			{
				if (IsConnected())
					m_connection->Send(msg);
			}

			// Получение очереди сообщений с сервера
			tsqueue<owned_message<T>>& Incoming()
			{
				return m_qMessagesIn;
			}

		protected:
			// передачей данных управляет контекст asio...
			asio::io_context m_context;
			// ...но нуждается в собственном потоке для выполнения своих рабочих команд
			std::thread thrContext;
			// У клиента есть единственный экземпляр объекта "connection", который обрабатывает передачу данных
			std::unique_ptr<connection<T>> m_connection;

		private:
			// Это потокобезопасная очередь входящих сообщений с сервера
			tsqueue<owned_message<T>> m_qMessagesIn;
		};



	}

}
