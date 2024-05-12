#pragma once

#include "net_common.h"
#include "nrt_tsqueue.h"
#include "net_message.h"


namespace olc {

	namespace net {

		template<typename T>
		class server_interface;

		template<typename T>
		class connection : public std::enable_shared_from_this<connection<T>>
		{
		public:
			// Соединение "принадлежит" либо серверу, либо клиенту, и его
 // поведение у них немного отличается.
			enum class owner
			{
				server,
				client
			};

		public:
			//Конструктор: Укажите владельца, подключитесь к контексту, передайте сокет
 // Укажите ссылку на очередь входящих сообщений
			connection(owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket, tsqueue<owned_message<T>>& qIn)
				: m_asioContext(asioContext), m_socket(std::move(socket)), m_qMessagesIn(qIn)
			{
				m_nOwnerType = parent;

				if (m_nOwnerType == owner::server) {

					random_device dev;
					mt19937 rng(dev());
					uniform_int_distribution<mt19937::result_type> dist(1, 1000000);
					// cout << dist(rng) << endl;
					int m_nHandShakeOut = (dist(rng));
					int m_nHandShakeCheck = scrable(m_nHandShakeOut);
				}
				else {
					int m_nHandShakeIn = 0;
					int m_nHandShakeOut = 0;

				}




			}

			virtual ~connection()
			{}

			// Этот идентификатор используется во всей системе - так клиенты будут понимать других клиентов
 // существует во всей системе.
			uint32_t GetID() const
			{
				return id;
			}

		public:
			void ConnectToClient(olc::net::server_interface<T>* server, uint32_t uid = 0)
			{
				if (m_nOwnerType == owner::server)
				{
					if (m_socket.is_open())
					{
						id = uid;
						//ReadHeader();

						WriteValidation();
						ReadValidation(server);


					}
				}
			}

			void ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
			{
				// Только клиенты могут подключаться к серверам
				if (m_nOwnerType == owner::client)
				{
					// Запрос asio пытается подключиться к конечной точке
					asio::async_connect(m_socket, endpoints,
						[this](std::error_code ec, asio::ip::tcp::endpoint endpoint)
						{
							if (!ec)
							{
								//ReadHeader();
								ReadValidation();
								
							}
						});
				}
			}


			void Disconnect()
			{
				if (IsConnected())
					asio::post(m_asioContext, [this]() { m_socket.close(); });
			}

			bool IsConnected() const
			{
				return m_socket.is_open();
			}

			// Настройте соединение на ожидание входящих сообщений
			void StartListening()
			{

			}

		public:
			// АСИНХРОННАЯ - отправка сообщения, соединения выполняются один к одному, поэтому указывать не нужно
 // цель, для клиента целью является сервер, и наоборот
			void Send(const message<T>& msg)
			{
				asio::post(m_asioContext,
					[this, msg]()
					{
						// Если в очереди есть сообщение, то мы должны
// предположить, что оно находится в процессе асинхронной записи.
 // В любом случае добавьте сообщение в очередь для вывода. Если сообщений нет
 // были доступны для записи, затем запустите процесс записи
						//сообщения  в начале очереди.
						bool bWritingMessage = !m_qMessagesOut.empty();
						m_qMessagesOut.push_back(msg);
						if (!bWritingMessage)
						{
							WriteHeader();
						}
					});
			}



		private:
			// АСИНХРОННЫЙ - основной контекст для написания заголовка сообщения
			void WriteHeader()
			{
				// Если эта функция вызвана, мы знаем, что в очереди исходящих сообщений должно быть 
 // по крайней мере одно сообщение для отправки. Поэтому выделите буфер передачи для хранения
// сообщения и выполните команду work - asio, отправив эти байты
				asio::async_write(m_socket, asio::buffer(&m_qMessagesOut.front().header, sizeof(message_header<T>)),
					[this](std::error_code ec, std::size_t length)
					{
						// asio уже отправил байты - если бы возникла проблема
 // было бы доступно сообщение об ошибке...
						if (!ec)
						{
							// ...ошибки нет, поэтому проверьте, есть ли в заголовке только что отправленного сообщения текст сообщения.
 //..
							if (m_qMessagesOut.front().body.size() > 0)
							{
								// ...это так, поэтому задайте задачу по записи байтов тела
								WriteBody();
							}
							else
							{
								// ...этого не произошло, поэтому мы закончили с этим сообщением. Удалите его из 
 // очереди исходящих сообщений
								m_qMessagesOut.pop_front();

								// Если очередь не пуста, нужно отправить еще несколько сообщений, поэтому
// сделайте так, чтобы это произошло, выдав задание на отправку следующего заголовка.
								if (!m_qMessagesOut.empty())
								{
									WriteHeader();
								}
							}
						}
						else
						{
							// ...asio не удалось отправить сообщение, мы могли бы проанализировать причину, но
// пока просто предположим, что соединение прервалось, закрыв
							//сокет . Когда в будущем попытка записи на этот клиент завершится неудачей из-за
							 // закрытого сокета, это будет исправлено.
							std::cout << "[" << id << "] Write Header Fail.\n";
							m_socket.close();
						}
					});
			}

			//АСИНХРОННЫЙ - основной контекст для написания текста сообщения
			void WriteBody()
			{
				// Если эта функция вызвана, только что был отправлен заголовок, и этот заголовок
 // указывает на то, что для этого сообщения существует текст. Заполните буфер передачи
// данными текста и отправьте его!
				asio::async_write(m_socket, asio::buffer(m_qMessagesOut.front().body.data(), m_qMessagesOut.front().body.size()),
					[this](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							// Отправка прошла успешно, поэтому мы завершаем работу с сообщением
 // и удаляем его из очереди
							m_qMessagesOut.pop_front();

							// Если в очереди все еще есть сообщения, то выполните задание на
// отправку заголовка следующих сообщений.
							if (!m_qMessagesOut.empty())
							{
								WriteHeader();
							}
						}
						else
						{
							// Ошибка отправки, описание смотрите в эквиваленте функции WriteHeader() :P
							std::cout << "[" << id << "] Write Body Fail.\n";
							m_socket.close();
						}
					});
			}

			// АСИНХРОННЫЙ - основной контекст, готовый к чтению заголовка сообщения
			void ReadHeader()
			{
				// Если эта функция вызвана, мы ожидаем, что asio будет ждать, пока не получит
 // достаточное количество байт для формирования заголовка сообщения. Мы знаем, что заголовки имеют фиксированный размер
 //, поэтому выделите буфер передачи, достаточно большой для его хранения. Фактически,
// мы создадим сообщение во "временном" объекте message, поскольку с ним 
 // удобно работать.
				asio::async_read(m_socket, asio::buffer(&m_msgTemporaryIn.header, sizeof(message_header<T>)),
					[this](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							// Был прочитан полный заголовок сообщения, проверьте, есть ли у этого сообщения текст, за которым можно следовать.
 //..
							if (m_msgTemporaryIn.header.size > 0)
							{
								// ...это так, поэтому выделите достаточно места в теле
// векторе сообщений и выдайте asio задание на чтение тела.
								m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.size);
								ReadBody();
							}
							else
							{
								// это не так, поэтому добавьте это сообщение без текста в список подключений
 // очередь входящих сообщений
								AddToIncomingMessageQueue();
							}
						}
						else
						{
							//При чтении формы на клиенте произошла ошибка, скорее всего, произошло отключение
 //. Закройте сокет и дайте системе возможность исправить это позже.
							std::cout << "[" << id << "] Read Header Fail.\n";
							m_socket.close();
						}
					});
			}

			// АСИНХРОННЫЙ - основной контекст, готовый к чтению текста сообщения
			void ReadBody()
			{
				// Если эта функция вызвана, заголовок уже прочитан, и этот заголовок
 // запрашивает, чтобы мы прочитали текст, пространство для этого текста уже выделено
 // во временном объекте сообщения, поэтому просто дождитесь поступления байтов...
				asio::async_read(m_socket, asio::buffer(m_msgTemporaryIn.body.data(), m_msgTemporaryIn.body.size()),
					[this](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{// ...и они это сделали! Теперь сообщение готово, поэтому добавьте
// все сообщение целиком во входящую очередь
							AddToIncomingMessageQueue();
						}
						else
						{
							// Как указано выше!
							std::cout << "[" << id << "] Read Body Fail.\n";
							m_socket.close();
						}
					});
			}

			// Как только будет получено полное сообщение, добавьте его во входящую очередь
			void AddToIncomingMessageQueue()
			{
				// Поместите его в очередь, преобразовав в "собственное сообщение", инициализировав
 // с помощью общего указателя из этого объекта connection
				if (m_nOwnerType == owner::server)
					m_qMessagesIn.push_back({ this->shared_from_this(), m_msgTemporaryIn });
				else
					m_qMessagesIn.push_back({ nullptr, m_msgTemporaryIn });

				//Теперь мы должны настроить контекст asio для получения следующего сообщения. Он
// будет просто сидеть и ждать поступления байтов, после чего построение сообщения
 // повторится. Умно, да?
				ReadHeader();
			}


			int scrable(int nInput) {
				return (nInput%10000);
			}

		    /*	WriteValidation();
			ReadValidation(server);*/
			void WriteValidation()
			{
				asio::async_write(m_socket, asio::buffer(&m_nHandShakeOut, sizeof(int)),
					[this](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							if (m_nOwnerType == owner::client)
								ReadHeader();
						}
						else
						{
							m_socket.close();
						}
					});
			}

			void ReadValidation(olc::net::server_interface<T>* server = nullptr)
			{

				asio::async_read(m_socket, asio::buffer(&m_nHandShakeIn, sizeof(int)),
					[this, server](std::error_code ec, std::size_t length)
					{
						if (!ec)
						{
							if (m_nOwnerType == owner::server)
							{
								if (m_nHandShakeIn == m_nHandShakeCheck)
								{
									cout << " Client Validated " << endl;
									server->OnClientValidated(this->shared_from_this());

									ReadHeader();

								}
								else
								{
									cout << " Not Coonection " << endl;
									m_socket.close();

								}

							}
							else
							{
								m_nHandShakeOut = scrable(m_nHandShakeIn);

								WriteValidation();

							}




						}
					});

			}





		protected:
			// Каждое соединение имеет уникальный разъем для подключения к удаленному компьютеру.
			asio::ip::tcp::socket m_socket;

			// Этот контекст является общим для всего экземпляра asio
			asio::io_context& m_asioContext;

			// В этой очереди хранятся все сообщения, которые должны быть отправлены удаленной стороне
 // этого соединения
			tsqueue<message<T>> m_qMessagesOut;

			// Это ссылается на входящую очередь родительского объекта
			tsqueue<owned_message<T>>& m_qMessagesIn;

			//Входящие сообщения создаются асинхронно, поэтому мы будем
 // сохранять здесь частично собранное сообщение до тех пор, пока оно не будет готово
			message<T> m_msgTemporaryIn;

			//"Владелец" решает, как будут вести себя некоторые соединения
			owner m_nOwnerType = owner::server;

			uint32_t id = 0;
			
			int m_nHandShakeOut = 0;
			int m_nHandShakeIn = 0;
			int m_nHandShakeCheck = 0;

		};



	}
}