#include <random>
#include <string>
#include <thread>
#include <chrono>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include "mqtt/async_client.h"

//-----------------------------------------------------------------------------

/* Configuration. */
const std::string BROKER = "tcp://rpi.mateuszkapala.eu:1883";
const std::string CLIENT_ID = "cpp_async_mqtt_client";
const std::string TOPIC = "example/cpp/random";
const int QOS = 1;
const bool RETAINED = false;
const auto TIMEOUT = std::chrono::seconds(10);
const int MAX_ACTIONS = 4;

/* Application flags. */
bool finished_publish = false;
bool finished_subscribe = false;

//-----------------------------------------------------------------------------

/* A callback class for use with the main MQTT client. */
class callback : public virtual mqtt::callback
{
public:
	/* Method called after successfully published messege. */
	void delivery_complete(mqtt::delivery_token_ptr tok) override 
	{
		static int cnt = 0;
		std::cout << "\t#" << ++cnt << " -> Delivery complete for token: "
			<< (tok ? tok->get_message_id() : -1) << std::endl;

		if (cnt >= MAX_ACTIONS)
			finished_publish = true;
	}

	/* Method called after message arrives at currently subscribed topic. */
	void message_arrived(mqtt::const_message_ptr msg) override 
	{
		static int cnt = 0;
		std::cout << "\t#" << ++cnt << " ->> Message arrived! " <<  "topic: '" << msg->get_topic() << "' " << 
		"payload: '" << msg->to_string() << "'\n" << std::endl;

		if (cnt >= MAX_ACTIONS)
			finished_subscribe = true;
	}

};

//-----------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	/* Create asynchronous client object that use non-blocking methods and
		allow us to run an operations in background. */
	mqtt::async_client client(BROKER, CLIENT_ID);
	mqtt::connect_options connOpts;
	connOpts.set_clean_session(true);
	connOpts.set_automatic_reconnect(true);

	callback cb;
	client.set_callback(cb);

	/* Create a topic object. This is a convenience since we will
		repeatedly publish messages with the same parameters. */
	mqtt::topic topic(client, TOPIC, QOS, RETAINED);

	/* Setup of RNG in range [0 - 100] */
	std::random_device rnd;							// TRNG
    std::mt19937 gen(rnd());						// Mersenne Twister 19937 PRNG
    std::uniform_int_distribution<> dis(0, 100);	// Specify numbers range

    try 
	{
		/* Connect to the MQTT broker. */
		std::cout << "Connecting to server '" << BROKER << "'... ";
		client.connect(connOpts)->wait();
		std::cout << "OK\n" << std::endl;

		/* Subscribe to topic on which we will be publishing messages. */
		client.subscribe(TOPIC, QOS);

		/* Main application loop. */
		while (!(finished_publish && finished_subscribe))
		{
			std::this_thread::sleep_for(std::chrono::seconds(2));

			if (!finished_publish)
			{
				/* Generate some data. */
				int x = dis(gen);

				/* Create some simple payload, ex. JSON object. */
				std::string payload = "{\"random\": \"" + std::to_string(x) + "\"}";
				std::cout << payload << std::endl;

				/* Publish to the topic. */
				topic.publish(payload);
			}
		}

		/* Disconnect from broker */
		std::cout << "\nDisconnecting... ";
		client.disconnect()->wait();
		std::cout << "OK" << std::endl;
	} 
	catch (const mqtt::exception& exc) 
	{
		std::cerr << exc.what() << std::endl;
		return 1;
	}

 	return 0;
}

