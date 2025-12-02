#include "Application.h"
#include <iostream>

int main() {
    Application app;
	std::cout << "Initializing application..." << std::endl;
    if (!app.initialize()) {
        std::cerr << "Failed to initialize application!" << std::endl;
        return -1;
    }
	std::cout << "Starting application..." << std::endl;
    app.run();
	std::cout << "Shutting down application..." << std::endl;
    app.shutdown();

    return 0;
}
