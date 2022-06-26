#include <asio2/mqtt/mqtt_client.hpp>
#include <iostream>
#include <asio2/external/fmt.hpp>
#include <asio2/mqtt/message.hpp>

int main()
{
	//std::string_view host = "broker.hivemq.com";
	std::string_view host = "127.0.0.1";
	std::string_view port = "1883";

	asio2::mqtt::message msg1(mqtt::v5::connect{});

	msg1 = mqtt::v5::subscribe{};

	[[maybe_unused]] mqtt::v5::subscribe       & subref1 = static_cast<      mqtt::v5::subscribe&>(msg1);
	[[maybe_unused]] mqtt::v5::subscribe const & subref2 = static_cast<const mqtt::v5::subscribe&>(msg1);
	[[maybe_unused]] mqtt::v5::subscribe         subvar3 = static_cast<      mqtt::v5::subscribe >(msg1);
	[[maybe_unused]] mqtt::v5::subscribe const   subvar4 = static_cast<const mqtt::v5::subscribe >(msg1);
	[[maybe_unused]] mqtt::v5::subscribe       * subptr1 = static_cast<      mqtt::v5::subscribe*>(msg1);
	[[maybe_unused]] mqtt::v5::subscribe const * subptr2 = static_cast<const mqtt::v5::subscribe*>(msg1);

	[[maybe_unused]] mqtt::v5::subscribe       & subrefa = msg1;

	asio2::mqtt_client client;

	mqtt::v5::connect conn;
	conn.username("3772738@qq.com");
	conn.password("0123456789123456");
	conn.properties(
		mqtt::v5::payload_format_indicator{ mqtt::v5::payload_format_indicator::format::string },
		mqtt::v5::session_expiry_interval{ 60 });
	conn.will_attributes(
		u8"0123456789abcdefg",
		u8"0123456789abcdefg",
		mqtt::qos_type::at_least_once, true,
		mqtt::v5::payload_format_indicator{ 0 },
		mqtt::v5::session_expiry_interval{ 60 });
	conn.will_attributes(
		u8"0123456789abcdefg",
		u8"0123456789abcdefg",
		mqtt::qos_type::at_least_once, true);

	std::vector<asio::const_buffer> sbuffer;
	std::string sdata;
	std::vector<std::uint8_t> v1data;
	std::vector<char> v2data;
	conn.serialize(sdata);
	conn.serialize(sbuffer);

	fmt::print("connect version : {}\n", int(conn.version()));

	char* p1 = (char*)&sbuffer[0];
	char* p2 = (char*)&sbuffer[sbuffer.size() - 1];

	auto ssss = sizeof(asio::const_buffer);
	auto size = p2 - p1;

	fmt::print("serialize to std::string                    , size : {}\n", sdata.size());
	fmt::print("serialize to std::vector<asio::const_buffer>, size : {}\n", size);
	fmt::print("sizeof(asio::const_buffer) : {}\n", ssss);
	fmt::print("connect.required_size()    : {}\n", conn.required_size());
	fmt::print("\n");

	client.post([]() {}, std::chrono::seconds(3));

	client.bind_recv([&](std::string_view data)
	{
		asio2::ignore_unused(data);

		//mqtt::v3::connack cack;
		//cack.deserialize(data);

		//mqtt::v4::connack cack;
		//cack.deserialize(data);

		//mqtt::v5::connack cack;
		//cack.deserialize(data);

		//cack.properties().has<mqtt::v5::receive_maximum>();

		//std::cout << "reason_code value is : " << int(cack.reason_code()) << std::endl;

		//mqtt::v5::receive_maximum* prm = cack.properties().get_if<mqtt::v5::receive_maximum>();
		//if (prm)
		//	std::cout << "receive_maximum value is : " << prm->value() << std::endl;

		//mqtt::v5::topic_alias_maximum* ptam = cack.properties().get_if<mqtt::v5::topic_alias_maximum>();
		//if(ptam)
		//	std::cout << "topic_alias_maximum value is : " << ptam->value() << std::endl;

		//mqtt::v5::reason_string* prs = cack.properties().get_if<mqtt::v5::reason_string>();
		//if(prs)
		//	std::cout << "reason_string value is : " << cack.reason_string() << std::endl;

		//if (cack.reason_code() == mqtt::v3::connect_reason_code::success)
		//	std::cout << "connect success-----------------------------\n";
		//else
		//	std::cout << "connect failure*****************************\n";

		//std::ignore = cack;

	}).bind_connect([&]()
	{
		if (asio2::get_last_error())
			printf("connect failure : %d %s\n",
				asio2::last_error_val(), asio2::last_error_msg().c_str());
		else
			printf("connect success : %s %u\n",
				client.local_address().c_str(), client.local_port());

		if (!asio2::get_last_error())
		{
			client.start_timer(1, 1000, 1, [&]()
			{
				mqtt::v5::subscribe sub;
				sub.packet_id(1001);
				sub.add_subscriptions(mqtt::subscription{ "/asio2/mqtt/index",mqtt::qos_type::at_least_once });
				client.async_send(std::move(sub), []()
				{
					std::cout << "send v5::subscribe, packet_id : 1001" << std::endl;
				});
			});

			client.start_timer(2, 2000, 10, [&]()
			{
				static int i = 1;

				mqtt::v5::publish pub;
				pub.qos(mqtt::qos_type::at_least_once);
				pub.packet_id(1002);
				pub.topic_name("/asio2/mqtt/index");
				pub.payload(fmt::format("{}", i++));
				client.async_send(std::move(pub), []()
				{
					std::cout << "send v5::publish  , packet_id : 1002" << std::endl;
				});
			});
		}

	}).bind_disconnect([]()
	{
		printf("disconnect : %d %s\n",
			asio2::last_error_val(), asio2::last_error_msg().c_str());
	});

	client.on_connack([](mqtt::v5::connack& connack)
	{
		fmt::print("recv v5::connack  , reason code: {}", int(connack.reason_code()));

		for (auto& vprop : connack.properties().data())
		{
			std::visit([](auto& prop)
			{
				[[maybe_unused]] auto name  = prop.name();
				[[maybe_unused]] auto type  = prop.type();
				[[maybe_unused]] auto value = prop.value();

				fmt::print(" {}:{}", name, value);
			}, vprop.variant());
		}
		fmt::print("\n");
	});

	client.on_suback([](mqtt::v5::suback& msg)
	{
		fmt::print("recv v5::suback   , packet_id : {} reason code: {}\n",
			msg.packet_id(), msg.reason_codes().at(0));
	});

	client.on_publish([](mqtt::v5::publish& msg, mqtt::message& rep)
	{
		asio2::ignore_unused(msg, rep);
		fmt::print("recv v5::publish  , packet id : {} QoS : {} topic_name : {} payload : {}\n",
			msg.packet_id(), int(msg.qos()), msg.topic_name(), msg.payload());
	});

	client.on_puback([](mqtt::v5::puback& puback)
	{
		fmt::print("recv v5::puback   , packet id : {} reason code : {}\n", puback.packet_id(), puback.reason_code());
	});

	client.on_pubrec([](mqtt::v5::pubrec& msg, mqtt::v5::pubrel& rep)
	{
		asio2::ignore_unused(msg, rep);
		fmt::print("recv v5::pubrec   , packet id: {} reason_code : {}\n",
			msg.packet_id(), int(msg.reason_code()));
	});

	client.on_pubcomp([](mqtt::v5::pubcomp& msg)
	{
		fmt::print("recv v5::pubcomp  , packet id: {} reason_code : {}\n",
			msg.packet_id(), int(msg.reason_code()));
	});

	client.on_pingresp([](mqtt::message& msg)
	{
		std::ignore = msg;
		printf("recv v5::pingresp\n");
	});

	client.on_disconnect([](mqtt::message& msg)
	{
		asio2::ignore_unused(msg);
		if (mqtt::v5::disconnect* p = static_cast<mqtt::v5::disconnect*>(msg); p)
			printf("recv v5::disconnect, reason code : %u\n", p->reason_code());
	});

	mqtt::v5::connect connect;
	connect.client_id(u8"37792738@qq.com");

	client.start(host, port, std::move(connect));

    [[maybe_unused]] auto cid = client.get_client_id();

	while (std::getchar() != '\n');

	return 0;
}
