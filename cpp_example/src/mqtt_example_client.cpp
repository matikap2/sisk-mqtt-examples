// #include <iostream>

// int main()
// {
//     std::cout << "Test CMake" << std::endl;

//     return 0;
// }

// #include <iostream>
// #include <cstdlib>
// #include <string>
// #include <thread>	// For sleep
// #include <atomic>
// #include <chrono>
// #include <cstring>
// #include "MQTTClient.h"

// #define ADDRESS     "tcp://rpi.mateuszkapala.eu:1883"
// #define CLIENTID    "ExampleClientPub"
// #define TOPIC       "/test"
// #define PAYLOAD     "Hello from the client C side!"
// #define QOS         0
// #define TIMEOUT     10000L

// int main(int argc, char* argv[])
// {
//     MQTTClient client;
//     MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
//     MQTTClient_message pubmsg = MQTTClient_message_initializer;
//     MQTTClient_deliveryToken token;
//     int rc;

//     if ((rc = MQTTClient_create(&client, ADDRESS, CLIENTID,
//         MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
//     {
//          printf("Failed to create client, return code %d\n", rc);
//          exit(EXIT_FAILURE);
//     }

//     conn_opts.keepAliveInterval = 20;
//     conn_opts.cleansession = 1;
//     if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
//     {
//         printf("Failed to connect, return code %d\n", rc);
//         exit(EXIT_FAILURE);
//     }

//     char* payload = PAYLOAD;
//     pubmsg.payload = payload;
//     pubmsg.payloadlen = (int)strlen(PAYLOAD);
//     pubmsg.qos = QOS;
//     pubmsg.retained = 0;
//     if ((rc = MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
//     {
//          printf("Failed to publish message, return code %d\n", rc);
//          exit(EXIT_FAILURE);
//     }

//     printf("Waiting for up to %d seconds for publication of %s\n"
//             "on topic %s for client with ClientID: %s\n",
//             (int)(TIMEOUT/1000), PAYLOAD, TOPIC, CLIENTID);
//     rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
//     printf("Message with delivery token %d delivered\n", token);

//     if ((rc = MQTTClient_disconnect(client, 10000)) != MQTTCLIENT_SUCCESS)
//     	printf("Failed to disconnect, return code %d\n", rc);
//     MQTTClient_destroy(&client);
//     return rc;
// }

#include <random>
#include <string>
#include <thread>
#include <chrono>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include "mqtt/async_client.h"

using namespace std;
using namespace std::chrono;

const std::string DFLT_ADDRESS { "tcp://rpi.mateuszkapala.eu:1883" };

const string TOPIC { "data/rand" };
const int	 QOS = 1;

const auto PERIOD = seconds(5);

const int MAX_BUFFERED_MSGS = 120;	// 120 * 5sec => 10min off-line buffering

const string PERSIST_DIR { "data-persist" };

/////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    try {
	string address = (argc > 1) ? string(argv[1]) : DFLT_ADDRESS;

	mqtt::async_client cli(address, "", MAX_BUFFERED_MSGS, PERSIST_DIR);

	mqtt::connect_options connOpts;
	connOpts.set_keep_alive_interval(MAX_BUFFERED_MSGS * PERIOD);
	connOpts.set_clean_session(true);
	connOpts.set_automatic_reconnect(true);

	// Create a topic object. This is a conventience since we will
	// repeatedly publish messages with the same parameters.
	mqtt::topic top(cli, TOPIC, QOS, true);

	// Random number generator [0 - 100]
	random_device rnd;
    mt19937 gen(rnd());
    uniform_int_distribution<> dis(0, 100);

	
		// Connect to the MQTT broker
		cout << "Connecting to server '" << address << "'..." << flush;
		cli.connect(connOpts)->wait();
		cout << "OK\n" << endl;

		char tmbuf[32];
		unsigned nsample = 0;

		// The time at which to reads the next sample, starting now
		auto tm = steady_clock::now();

		while (true) {
			// Pace the samples to the desired rate
			this_thread::sleep_until(tm);

			// Get a timestamp and format as a string
			time_t t = system_clock::to_time_t(system_clock::now());
			strftime(tmbuf, sizeof(tmbuf), "%F %T", localtime(&t));

			// Simulate reading some data
			int x = dis(gen);

			// Create the payload as a text CSV string
			string payload = to_string(++nsample) + "," +
								tmbuf + "," + to_string(x);
			cout << payload << endl;

			// Publish to the topic
			top.publish(std::move(payload));

			tm += PERIOD;
		}

		// Disconnect
		cout << "\nDisconnecting..." << flush;
		cli.disconnect()->wait();
		cout << "OK" << endl;
	}
	catch (const mqtt::exception& exc) {
		cerr << exc.what() << endl;
		return 1;
	}

 	return 0;
}

