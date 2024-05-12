#pragma once
 /*#include <iostream>
//#ifdef _WIN32
//#define _WIN32_WINNT 0*0A00
//#endif
#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <thread>
//#include <boost/asio.hpp>
//#include <boost/asio/ts/buffer.hpp>
//#include <boost/asio/ts/internet.hpp>
using namespace std;

vector<char> vbuffer(20 * 1024); //20 * 1024
int perem = 0;

void GrabSameDate(asio::ip::tcp::socket& socket) {

	socket.async_read_some(asio::buffer(vbuffer.data(), vbuffer.size()),
		[&](error_code ec, size_t length) {
			if (!ec) {

				cout << "n\nREad " << length << " bytes\n\n";
				for (int i = 0; i < length; i++)
				{
					cout << vbuffer[i];
				}
				/*cout  << endl;
				++perem;
				cout << perem << endl;*/
				/*	GrabSameDate(socket);
				}



			});

	}
	*/

	/*
	int main() {


		asio::error_code ec;

		asio::io_context context;

		asio::io_context::work idwork(context);

		thread theContext = thread([&]() { context.run(); });

		asio::ip::tcp::endpoint endp(asio::ip::make_address("93.184.216.34", ec), 80); //93.184.216.34

		asio::ip::tcp::socket sct(context);

		sct.connect(endp, ec);

		if (!ec) {
			cout << "connect" << endl;
		}
		else {
			cout << ec.message() << endl;
		}

		if (sct.is_open()) {
			GrabSameDate(sct);
			string sRequest = "GET " "Host " "Connection";
			sct.write_some(asio::buffer(vbuffer.data(), vbuffer.size()), ec);

			this_thread::sleep_for(chrono::milliseconds(2000));
			context.stop();
			if (theContext.joinable())theContext.join();

		}




		return 0;




	}
	*/