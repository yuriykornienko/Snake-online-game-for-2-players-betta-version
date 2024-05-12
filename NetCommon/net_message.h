#pragma once
#include "net_common.h"

namespace olc
{

	namespace net
	{
		template <typename T>
		struct message_header
		{
			T id{};
			uint32_t size = 0;
		};

		// Текст сообщения содержит заголовок и std::vector, содержащий необработанные байты
 // информации. Таким образом, длина сообщения может быть переменной, но размер
 // в заголовке должен быть изменен.
		template <typename T>
		struct message
		{
			// Header & Body vector
			message_header<T> header{};
			std::vector<uint8_t> body;

			//возвращает размер всего пакета сообщений в байтах
			size_t size() const
			{
				return body.size();
			}

			// Переопределение для совместимости с std::cout - создает понятное описание сообщения
			friend std::ostream& operator << (std::ostream& os, const message<T>& msg)
			{
				os << "ID:" << int(msg.header.id) << " Size:" << msg.header.size;
				return os;
			}

			// Удобные перегрузки операторов - они позволяют нам добавлять и удалять данные из
 // вектора body, как если бы это был стек, то есть сначала вводим, а затем выводим. Это
			//сам по себе шаблон , потому что мы не знаем, какой тип данных вводит пользователь
				//, поэтому разрешаем использовать их все.ПРИМЕЧАНИЕ: Предполагается, что тип данных по сути является
				// Обычными старыми данными (POD). TLDR: Сериализация и десериализация в/из вектора

			//Помещает любые данные, подобные POD, в буфер сообщений
			template<typename DataType>
			friend message<T>& operator << (message<T>& msg, const DataType& data)
			{
				//Убедитесь, что тип передаваемых данных поддается простому копированию
				static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector");

				//Кэшируйте текущий размер вектора, так как это будет точка, в которую мы вставим данные
				size_t i = msg.body.size();

				//Измените размер вектора в соответствии с размером передаваемых данных
				msg.body.resize(msg.body.size() + sizeof(DataType));

				//Физически скопируйте данные во вновь выделенное векторное пространство
				std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

				//Пересчитайте размер сообщения
				msg.header.size = msg.size();

				// Верните целевое сообщение, чтобы его можно было "привязать".
				return msg;
			}

			// Извлекает любые данные, подобные POD, из буфера сообщений
			template<typename DataType>
			friend message<T>& operator >> (message<T>& msg, DataType& data)
			{
				// Убедитесь, что тип передаваемых данных поддается простому копированию
				static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pulled from vector");

				// Кэшируйте местоположение ближе к концу вектора, где начинаются извлекаемые данные
				size_t i = msg.body.size() - sizeof(DataType);

				//Физически скопируйте данные из вектора в пользовательскую переменную
				std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

				// Уменьшите вектор, чтобы удалить прочитанные байты, и сбросьте конечную позицию
				msg.body.resize(i);

				//Пересчитайте размер сообщения
				msg.header.size = msg.size();

				//Верните целевое сообщение, чтобы его можно было "привязать".
				return msg;
			}
		};


		// "Принадлежащее" сообщение идентично обычному сообщению, но оно связано с
 // соединением. На сервере владельцем будет клиент, отправивший сообщение,
// на клиенте владельцем будет сервер.

		//Переслать объявление о соединении
		template <typename T>
		class connection;

		template <typename T>
		struct owned_message
		{
			std::shared_ptr<connection<T>> remote = nullptr;
			message<T> msg;

			// И снова дружелюбный производитель струн //Again, a friendly string maker
			friend std::ostream& operator<<(std::ostream& os, const owned_message<T>& msg)
			{
				os << msg.msg;
				return os;
			}
		};

	}

}