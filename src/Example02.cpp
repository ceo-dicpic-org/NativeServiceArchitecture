#include <iostream>
#include <cstring>

#include "Service.hpp"

enum Name
{
	Alfred = 0,
	Bernard,
	Caesar,
	Dominic,
	Eric,
	Fiona,
	Gretel,
	Heinrich,
	Iwi,
	Jane,
	Karl,
	Lisa,
	Manuel,
	Nora,
	Otto,
	Pete,
	Quasimodo,
	Ronaldo,
	Stefania,
	Tina,
	Ulf,
	Verona,
	Willhelm,
	Xavier,
	Zlatko,
	TOTAL_NAME_COUNT
};

std::string name2string(const Name &name)
{
	switch (name)
	{
		case Alfred: return "Alfred";
		case Bernard: return "Bernard";
		case Caesar: return "Caesar";
		case Dominic: return "Dominic";
		case Eric: return "Eric";
		case Fiona: return "Fiona";
		case Gretel: return "Gretel";
		case Heinrich: return "Heinrich";
		case Iwi: return "Iwi";
		case Jane: return "Jane";
		case Karl: return "Karl";
		case Lisa: return "Lisa";
		case Manuel: return "Manuel";
		case Nora: return "Nora";
		case Otto: return "Otto";
		case Pete: return "Pete";
		case Quasimodo: return "Quasimodo";
		case Ronaldo: return "Ronaldo";
		case Stefania: return "Stefania";
		case Tina: return "Tina";
		case Ulf: return "Ulf";
		case Verona: return "Verona";
		case Willhelm: return "Willhelm";
		case Xavier: return "Xavier";
		case Zlatko: return "Zlatko";
	}
}

class Customer
{
public:
	Customer()
	{
		name = name2string(static_cast<Name>(rand() % TOTAL_NAME_COUNT));
	}

	std::string name; /// Each customer should have a name.
};





class Barber : public NSA::Service
{
public:
	Barber() : Service("Barber service", 1), cashRegister(1)
	{
		jobTimeOut(std::chrono::milliseconds(3000000));
	}

	Service::Future<void> sitOnChair(Customer customer)
	{
		NSA_MAKE_PROMISE(Barber::sitOnChairImp, void, customer);
	}

private:
	void sitOnChairImp(Service::Promise<void> promise, Customer customer)
	{
		printf("%s getting hair cut\n", customer.name.c_str());
		std::this_thread::sleep_for(std::chrono::seconds((rand() % 3) + 4));


		std::lock_guard<std::mutex> lock(cashMutex);

		if (cashRegister.push(customer))
			return;
		else
		{
			cashRegister.pop(&cashOut);
			printf("%s paid and leaves\n", cashOut.name.c_str());
			cashRegister.push(customer);
		}	
	}

	NSA::BlockingQueue<Customer> cashRegister;
	Customer cashOut;
	mutable std::mutex cashMutex;
};





class Sofa : public NSA::Service
{
public:
	Sofa(Barber &barber) : Service("Sofa service", 3),
		barber(barber)
	{
		jobTimeOut(std::chrono::milliseconds(3000000));
	}

	Service::Future<void> sitOnSofa(Customer customer)
	{
		NSA_MAKE_PROMISE(Sofa::sitOnSofaImp, void, customer);
	}

private:
	
	void sitOnSofaImp(Service::Promise<void>, Customer customer)
	{
		printf("%s sits on sofa.\n", customer.name.c_str());
		barber.sitOnChair(customer);
	}

	Barber &barber; ///< A reference to a barber service.
};






class Standing : public NSA::Service
{
public:
	Standing(Sofa &sofa) : Service("Standing service", 12), sofa(sofa)
	{
		jobTimeOut(std::chrono::milliseconds(3000000));
	}

	Service::Future<void> enterShop(Customer customer)
	{
		NSA_MAKE_PROMISE(Standing::enterShopImp, void, customer);
	}

private:
	void enterShopImp(Service::Promise<void> promise, Customer customer)
	{
		printf("%s enters the shop.\n", customer.name.c_str());
		sofa.sitOnSofa(customer);
	}

	Sofa &sofa; ///< A reference to a sofa service.
};





class Customers : public NSA::Service
{
public:
	Customers(Standing &standing) : Service("Customer generation service"), standing(standing)
	{}

	Service::Future<bool> simulateCustomers()
	{
		NSA_MAKE_PROMISE(Customers::produceCustomers, bool);
	}

private:
	void produceCustomers(Service::Promise<bool> promise)
	{
		const int total = 100;

		printf("Generating customers\n");
		for (std::size_t customer = 0; customer < total; ++customer)
		{
			standing.enterShop(Customer());
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}

		promise->set_value(true);
	}

	Standing &standing; ///< A reference to a standing service.
};

int main(int argc, char **argv)
{
	Barber barber;
	Sofa sofa(barber);
	Standing standing(sofa);
	Customers customers(standing);

	barber.detach(3);
	sofa.detach();
	standing.detach();
	customers.detach();

	NSA::Service::Future<bool> simulationEnd = customers.simulateCustomers();
	std::future_status status;

	simulationEnd->wait();

	customers.join();
	standing.join();
	sofa.join();
	barber.join();

	return EXIT_SUCCESS;
}
