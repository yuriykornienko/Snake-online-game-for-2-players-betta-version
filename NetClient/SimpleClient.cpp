#include <iostream>
#include <old_net.h>


int vvod_cursor;

enum class CustomMsgTypes : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage,
	Cursor,
};



class CustomClient : public olc::net::client_interface<CustomMsgTypes>
{
public:

	void MyFanctoin(int x)
	{
		olc::net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::Cursor;
		msg <<x;
		Send(msg);
	}

	void PingServer()
	{
		olc::net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerPing;

		// Будьте осторожны с этим.
		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

		msg << timeNow;
		Send(msg);
	}

	void MessageAll()
	{
		olc::net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::MessageAll;
		
		Send(msg);
	}
};

int main()
{
	CustomClient c;
	c.Connect("127.0.0.1", 60000);

	bool key[4] = { false, false, false, false };
	bool old_key[4] = { false, false, false, false };
	//int vvod_cursor;
	bool bQuit = false;
	while (!bQuit)
	{
		if (GetForegroundWindow() == GetConsoleWindow())
		{
			/*key[0] = GetAsyncKeyState('1') & 0x8000;
			key[1] = GetAsyncKeyState('2') & 0x8000;
			key[2] = GetAsyncKeyState('3') & 0x8000;*/
			key[0] = GetAsyncKeyState(VK_UP) & 0x8000;
			key[1] = GetAsyncKeyState(VK_LEFT) & 0x8000;
			key[2] = GetAsyncKeyState(VK_RIGHT) & 0x8000;
			key[3] = GetAsyncKeyState(VK_DOWN) & 0x8000;
		}
		//int vvod_cursor;

		if (key[0] && !old_key[0]) 
		{
			vvod_cursor = 119;  c.MyFanctoin(119);  //c.PingServer(119);
		}

		if (key[1] && !old_key[1])
		{
			vvod_cursor = 97;  c.MyFanctoin(97);// c.MessageAll(97);
		}
		if (key[2] && !old_key[2])
		{
			vvod_cursor = 100;  c.MyFanctoin(100); //c.MessageAll(100);
		}

		if (key[3] && !old_key[3])
		{
			vvod_cursor = 115; c.MyFanctoin(115);// c.MessageAll(115);
		}


		for (int i = 0; i < 4; i++) old_key[i] = key[i];

		if (c.IsConnected())
		{
			if (!c.Incoming().empty())
			{


				auto msg = c.Incoming().pop_front().msg;

				switch (msg.header.id)
				{
				case CustomMsgTypes::Cursor:
				{

					//std::cout << "123\n";
					//cout << x << endl;
					uint32_t clientID;
					//msg << vvod_cursor;
					msg >> clientID; //vvod_cursor;

					std::cout <<  clientID << "\n";
				}
				break;



				case CustomMsgTypes::ServerAccept:
				{

					std::cout << "Server Accepted Connection\n";
				}
				break;


				case CustomMsgTypes::ServerPing:
				{

					std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
					std::chrono::system_clock::time_point timeThen;
					msg >> timeThen;
					std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << "\n";
				}
				break;

				case CustomMsgTypes::ServerMessage:
				{

					uint32_t clientID;
					//msg << vvod_cursor;
					msg >>  clientID; //vvod_cursor;

					std::cout << "Hello from [" << clientID << "]\n";
				}
				break;
				}
			}
		}
		else
		{
			std::cout << "Server Down\n";
			bQuit = true;
		}

	}

	return 0;
};