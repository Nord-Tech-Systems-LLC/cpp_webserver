/*
** showip.c -- show IP addresses for a host given on the command line
*/

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <iostream>

// Helper function you can use:
// Convert a struct sockaddr address to a string, IPv4 and IPv6:
char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen) {
    switch (sa->sa_family) {
        case AF_INET:
            inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
                      s, maxlen);
            break;

        case AF_INET6:
            inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
                      s, maxlen);
            break;

        default:
            strncpy(s, "Unknown AF", maxlen);
            return NULL;
    }

    return s;
}

int main() {
    /**
     * IPv4 demo of inet_ntop() and inet_pton()
     * */
    struct sockaddr_in sa;
    char str[INET_ADDRSTRLEN];

    // store this IP address in sa:
    std::cout << "sa4 INET_PTON: " << inet_pton(AF_INET, "192.0.2.33", &(sa.sin_addr)) << std::endl;

    // now get it back and print it
    std::cout << "sa4 INET_NTOP: " << *inet_ntop(AF_INET, &(sa.sin_addr), str, INET_ADDRSTRLEN) << std::endl;
    printf("IPv4 address: %s\n", str);  // prints "192.0.2.33"

    /**
     * IPv6 demo of inet_ntop() and inet_pton()
     * */
    // (basically the same except with a bunch of 6s thrown around)
    struct sockaddr_in6 sa6;
    char str6[INET6_ADDRSTRLEN];

    // store this IP address in sa:
    std::cout << "sa6 INET_PTON: " << inet_pton(AF_INET6, "2001:db8:8714:3a90::12", &(sa6.sin6_addr)) << std::endl;

    // now get it back and print it -- RETURNS A POINTER, need to dereference it
    std::cout << "sa6 INET_NTOP: " << *inet_ntop(AF_INET6, &(sa6.sin6_addr), str6, INET6_ADDRSTRLEN) << std::endl;
    printf("IPv6 address: %s\n", str6);  // prints "2001:db8:8714:3a90::12"

    /**
     * other
     */
    struct addrinfo hints, *res;
    int sockfd;
    // first, load up address structs with getaddrinfo():
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    std::cout << "hints.ai_family: " << hints.ai_family << "\n";
    std::cout << "hints.ai_socktype: " << hints.ai_socktype << "\n";
    std::cout << "hints.ai_flags: " << hints.ai_flags << "\n";
    // std::cout << hints.ai_family << std::endl;

    // fill in my IP for me
    getaddrinfo(NULL, "3490", &hints, &res);

    // make a socket:
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    // bind it to the port we passed in to getaddrinfo():
    bind(sockfd, res->ai_addr, res->ai_addrlen);
    listen(sockfd, 10);  // set s up to be a server (listening) socket

    // then have an accept() loop down here somewhere
    return 0;
}