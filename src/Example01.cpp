#include <iostream>

#include "Service.hpp"

/**
 * @brief Icecream vendor service implementation.
 * @details The icecream vendor is a service that serves customers
 * icecream depending on their flavor choice.
 */
class IcecreamVendor : public NSA::Service
{
public:
	IcecreamVendor() : Service("Icecream Service")
	{}

private:
	void serveIcecreamImpl(Service::Promise<std::string> promise, const std::string order)
	{
		printf("%s: Working on order (%s)\n", name.c_str(), order.c_str());
		const int duration = (rand() % 4) + 3;
		std::this_thread::sleep_for(std::chrono::seconds(duration));
		std::string compositeOrder = "Cone with: " + order;
		promise->set_value(compositeOrder);
		printf("%s: Finished order (%s). Took %d minutes\n", name.c_str(), order.c_str(), duration);
	}

public:
	/**
	 * @brief Serves a customer icecream.
	 * @details The average duration of the service is 3 to 6 minuts.
	 * 
	 * @param order A string representation of the order.
	 * @return A future containing icecream.
	 */
	Service::Future<std::string> serveIcecream(const std::string order)
	{
		NSA_MAKE_PROMISE(IcecreamVendor::serveIcecreamImpl, std::string, order);
	}
};

/// A enum of randomflavors.
enum Flavor
{
	STRAWBERRY = 0,
	VANILLE,
	CHOCOLATE,
	BANANA,
	MILK,
	CHERRY,
	PRUNE,
	PEACH,
	LEMON,
	KIWI,
	CREAM,
	PINEAPPLE,
	APPLE,
	BACON,
	DURIAN,
	MELON,
	PITAYA,
	AVOCADO,
	CARROT,
	GARLIC,
	MANGO,
	ORANGE,
	PEAR,
	TOMATO,
	TOTAL_FLAVOR_COUNT
};

/**
 * @brief Simple function to convert a enum value to a string.
 * @param flavor A valid enum value of the flavor enum.
 * @return A string containing the flavor.
 */
std::string flavor2string(const Flavor &flavor)
{
	switch (flavor)
	{
		case STRAWBERRY: return "Strawberry"; 		
		case VANILLE:	 return "Vanille";	
		case CHOCOLATE:	 return "Chocolate";	
		case BANANA:	 return "Banana";	
		case MILK:		 return "Milk";
		case CHERRY:	 return "Cherry";	
		case PRUNE:		 return "Prune";
		case PEACH:		 return "Peach";
		case LEMON:		 return "Lemon";
		case KIWI:		 return "Kiwi";
		case CREAM:		 return "Cream";
		case PINEAPPLE:	 return "Pineapple";	
		case APPLE:		 return "Apple";
		case BACON:		 return "Bacon";
		case DURIAN:	 return "Durian";	
		case MELON:		 return "Melon";
		case PITAYA:	 return "Pitaya";	
		case AVOCADO:	 return "Avocado";	
		case CARROT:	 return "Carrot";	
		case GARLIC:	 return "Garlic";	
		case MANGO:		 return "Mango";
		case ORANGE:	 return "Orange";	
		case PEAR:		 return "Pear";
		case TOMATO :	 return "Tomato";	
	}
}

/**
 * @brief A function which creates a random flavor.
 * @return A full string containing a random flavor.
 */
std::string createRandomFlavor()
{
	std::string returnValue = "";
	
	const std::size_t flavors = (rand() % 5) + 1;

	for (std::size_t i = 0; i < flavors; i++)
	{
		returnValue += flavor2string(static_cast<Flavor>(rand() % TOTAL_FLAVOR_COUNT));

		if (i < flavors - 1) returnValue += " ";
	}

	return returnValue;
}

/**
 * @brief Thread function which generates customers.
 * @details This threaded function creates customers
 * based on a regulated model for the icecream service
 * simulation.
 * 
 * @param vendor A reference to the vendor to which send the customers.
 * @param done A status message to indicate the end of the simulation.
 */
void customers(IcecreamVendor &vendor, bool &done)
{
	// Define intervals of cusomters.
	const int morning = 7;
	const int midday  = 3;
	const int evening = 10;

	printf("Morning customers\n");
	for (std::size_t group = 0; group < 10; group++)
	{
		const std::size_t customers = (rand() % 3) + 1;
		std::this_thread::sleep_for(std::chrono::seconds(morning));

		printf("A group arives containing %d customers.\n", customers);

		for (std::size_t customer = 0; customer < customers; customer++)
			vendor.serveIcecream(createRandomFlavor());
	}

	printf("Midday customers\n");
	for (std::size_t group = 0; group < 10; group++)
	{
		const std::size_t customers = (rand() % 3) + 3;
		std::this_thread::sleep_for(std::chrono::seconds(midday));

		printf("A group arives containing %d customers.\n", customers);

		for (std::size_t customer = 0; customer < customers; customer++)
			vendor.serveIcecream(createRandomFlavor());
	}

	printf("Evening customers\n");
	for (std::size_t group = 0; group < 10; group++)
	{
		const std::size_t customers = (rand() % 4) + 3;
		std::this_thread::sleep_for(std::chrono::seconds(evening));

		printf("A group arives containing %d customers.\n", customers);

		for (std::size_t customer = 0; customer < customers; customer++)
			vendor.serveIcecream(createRandomFlavor());
	}	

	done = true;
}

int main(int argc, char **argv)
{
	// Create a icecream vendor.
	IcecreamVendor vendor;

	// Add 3 workers to the vendor.
	vendor.detach(3);

	// Set the wait condition.
	bool done = false;

	// Send in the customers.
	printf("The store is open\n");
	std::thread customerThread(&customers, std::ref(vendor), std::ref(done));

	// Print out overview information.
	while (!done)
	{
		printf("Current waiting customers: %d. Total customers: %d\n",
			vendor.currentJobs(), vendor.totalJobs());
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	printf("Store is closed, working of the final jobs.\n");

	// Join the costumer producing thread.
	customerThread.join();

	// Join the costumer cnsuming thread.
	vendor.join();

	printf("A total of %d customers where served today.\n", vendor.totalJobs());
	return EXIT_SUCCESS;
}