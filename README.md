This project has been created as part of the 42 curriculum by <ael-majd>, <yazlaigi>, <mourhouc>.

# Description

Webserv is a custom HTTP server implemented in C++98 as part of the 42 curriculum. The goal of the project is to understand how web servers work internally by building one from scratch without relying on existing server frameworks.

The server listens for incoming TCP connections, parses HTTP requests, processes them according to its configuration, and returns appropriate HTTP responses. It supports multiple clients simultaneously using non-blocking sockets and an event-driven architecture.

- HTTP/1.1 request handling
- Support for multiple server blocks
- Configurable host and port
- GET, POST, and DELETE methods
- Static file serving
- Directory listing (Autoindex)
- File uploads
- CGI execution (e.g. Python and Bash scripts)
- Custom error pages
- Configurable client body size
- Location-based routing
- Multiple simultaneous client connections using poll()
- Non-blocking sockets

# Instructions

## Requirements
    - C++98 compatible compiler
    - make
    - Unix-like operating system (Linux or macOS)
## Compilation
```bash
    make
```
### Run

```bash
./webserv <config_file>
```

Example:

```bash
./webserv config.conf
```
#### run cgi
make sure you define loction cgi on config file !!
```bash
    localhost:8080/cgi/here_your_script
```
### upload files
Go to:
```
localhost:8080/upload.html
```
## Resources

* RFC 7230 and RFC 7231 (HTTP/1.1)
* POSIX socket programming documentation
* C++98 documentation
* `poll(2)` and `socket(2)` manual pages

### AI Usage

AI tools were used to explain networking concepts, HTTP behavior, C++98 features, and to assist with debugging ideas and documentation. The project design, implementation, testing, and final validation were completed by the authors.
