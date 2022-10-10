#include <asio2/udp/udp_client.hpp>
#include <iostream>

int main()
{
	std::string_view host = "127.0.0.1";
	std::string_view port = "8036";

	asio2::udp_client client;

	client.bind_connect([&]()
	{
		if (asio2::get_last_error())
			printf("connect failure : %d %s\n",
				asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("connect success : %s %u\n",
				client.local_address().c_str(), client.local_port());

		// has no error, it means connect success, we can send data at here
		if (!asio2::get_last_error())
		{
			client.async_send("1<abcdefghijklmnopqrstovuxyz0123456789>");
		}

	}).bind_disconnect([]()
	{
		printf("disconnect : %d %s\n",
			asio2::last_error_val(), asio2::last_error_msg().c_str());

	}).bind_recv([&](std::string_view data)
	{
		printf("recv : %zu %.*s\n", data.size(), (int)data.size(), data.data());

		std::string s;
		s += '<';
		int len = 33 + std::rand() % (126 - 33);
		for (int i = 0; i < len; i++)
		{
			s += (char)((std::rand() % 26) + 'a');
		}
		s += '>';

		client.async_send(std::move(s));

	}).bind_handshake([&]()
	{
		if (asio2::get_last_error())
			printf("handshake failure : %d %s\n",
				asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("handshake success : %d %s\n",
				asio2::last_error_val(), asio2::last_error_msg().c_str());

		// this send will be failed, because connection is not fully completed
		client.async_send("abc", []()
		{
			ASIO2_ASSERT(asio2::get_last_error());
			std::cout << "send failed : " << asio2::last_error_msg() << std::endl;
		});
	});

	// to use kcp, the last param must be : asio2::use_kcp
	client.async_start(host, port, asio2::use_kcp);

	while (std::getchar() != '\n');

	client.stop();

	return 0;
}
