#include "CGI.hpp"
#include <unistd.h>

std::string CGI::execute(const std::string& scriptPath,const std::string& method,const std::string& body,
                        const std::map<std::string, std::string>& headers){
    int inPipe[2];
    int outPipe[2];
    pipe(inPipe);
    pipe(outPipe);

    pid_t pid = fork();
    if (pid == -1)
        throw std::runtime_error ("fork failed");
    if (pid == 0){
        dup2(inPipe[0], STDIN_FILENO);
        dup2(outPipe[1], STDOUT_FILENO);
        close(inPipe[0]);
        close(outPipe[1]);
        close(inPipe[1]);
        close(outPipe[0]);
        char *argv[] = {(char*)scriptPath.c_str(), NULL};
        std::vector<std::string> env;
        env.push_back("REQUEST_METHOD=" + method);
        env.push_back("CONTENT_LENGTH=" + std::to_string(body.size()));
        env.push_back("GATEWAY_INTERFACE=CGI/1.1");
        env.push_back("SERVER_PROTOCOL=HTTP/1.1");
        env.push_back("REDIRECT_STATUS=200");
        std::vector<char*> envp;
        for (size_t i = 0; i < env.size(); i++)
            envp.push_back((char*)env[i].c_str());
        envp.push_back(NULL);
        execve(scriptPath.c_str(), argv, envp.data());
        exit(1);
    }
    else{
        close(inPipe[0]);
        close(outPipe[1]);
        if (method == "POST")
            write (inPipe[1], body.c_str(), body.size());
        close (inPipe[1]);
        char buffer[4096];
        std::string output;
        ssize_t bytes;
        while ((bytes = read(outPipe[0], buffer, sizeof(buffer))) > 0)
            output.append(buffer,bytes);
        close (outPipe[0]);
        waitpid(pid, NULL, 0);
        return output;
    }
}