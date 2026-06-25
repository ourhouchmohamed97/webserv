This project has been created as part of the 42 curriculum by <ael-majd>, <yazlaigi>, <mourhouc>.

# Description

Webserv is a custom HTTP server implemented in C++98 as part of the 42 curriculum. The goal of the project is to understand how web servers work internally by building one from scratch without relying on existing server frameworks.

The server listens for incoming TCP connections, parses HTTP requests, processes them according to its configuration, and returns appropriate HTTP responses. It supports multiple clients simultaneously using non-blocking sockets and an event-driven architecture.


# Instructions

## Requirements
    - C++98 compatible compiler
    - make
    - Unix-like operating system (Linux or macOS)
## Compilation
### Run

```bash
./webserv <config_file>
```

Example:

```bash
./webserv config.conf
```

## Resources

* RFC 7230 and RFC 7231 (HTTP/1.1)
* POSIX socket programming documentation
* C++98 documentation
* `poll(2)` and `socket(2)` manual pages

### AI Usage

AI tools were used to explain networking concepts, HTTP behavior, C++98 features, and to assist with debugging ideas and documentation. The project design, implementation, testing, and final validation were completed by the authors.
