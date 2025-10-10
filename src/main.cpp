#include "Server.hpp"

int main(int ac, char**av)
{
    
    if (ac != 3){
        cerr << "Usage: " << av[0] << " <port> <password>" << endl;
        return EXIT_FAILURE;
    }

    const char* s = av[1];
    char* end = NULL;
    errno = 0;
    long x = strtol(s, &end, 10);
    
    if (end == s){
        cerr << "Port is not a number: " << s << endl;
        return EXIT_FAILURE;
    }
    
     if (end == s) {
        std::cerr << "Port is not a number: " << s << std::endl;
        return EXIT_FAILURE;
    }
    if (*end != '\0') {
        std::cerr << "Trailing non-numeric characters after number: " << end << std::endl;
        return EXIT_FAILURE;
    }

    // check overflow or underflow + port range
    if (errno == ERANGE || x < 0 || x > 65535) {
        std::cerr << "Port out of range: " << s << std::endl;
        return EXIT_FAILURE;
    }


    int port = static_cast<int>(x);
    cout << "port: " << port << endl;
    
    string serverPassword(av[2]);
    if (serverPassword.empty()){
        std::cerr << "Password cannot be empty\n";
        return EXIT_FAILURE;
    }
    // should I put limit to the password length ? :c

    Server server(port, serverPassword);
    
    try {
        // set up server
        server.serverStart();

        // start loop waiting for connections
    } catch (const Server::ServerError& e) {
        std::cerr << "Fatal Server Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (const std::bad_alloc& e) { // Catch out-of-memory specifically
        std::cerr << "Out of memory: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (const std::exception& e) { // Catch any other standard exceptions
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) { // Catch any other unknown exceptions
        std::cerr << "An unknown fatal error occurred." << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
