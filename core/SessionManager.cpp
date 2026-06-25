#include "SessionManager.hpp"

std::map<std::string, int> SessionManager::_sessions;

std::string SessionManager::generateId()
{
    static bool avoid_reuse = false;

    if (!avoid_reuse)
    {
        std::srand(time(NULL));
        avoid_reuse = true;
    }
    std::stringstream ss;
    ss << std::rand();
    return ss.str();
}

std::string SessionManager::get_cookies_val(const std::string &cookies_header, const std::string &key)
{
    size_t pos = cookies_header.find(key);
    if (pos == std::string::npos)
        return "";
    pos += key.length();
    size_t end = cookies_header.find(";", pos);
    if (end == std::string::npos)
        end = cookies_header.length();
    return cookies_header.substr(pos, end - pos);
}

HttpResponse SessionManager::handle(const HttpRequest &req)
{
    HttpResponse res;


    res.statusCode = 200;
    res.headers["Content-type"] = "text/html";
    
    std::string session_id;
    std::map<std::string, std::string>::const_iterator it = req.headers.find("Cookie");

    if (it != req.headers.end())
        session_id = get_cookies_val(it->second, "session_id=");
    
    if (session_id.empty() || _sessions.find(session_id) == _sessions.end())
    {
        session_id = generateId();
        _sessions[session_id] = 1;

        res.headers["Set-Cookie"] = "session_id=" + session_id;
        res.body = "<html><body>"
                    "<h1>New session created</h1>"
                    "<p>Visits No: 1</p>"
                    "</body></html>";
    }
    else
    {
       _sessions[session_id]++;
        std::stringstream ss;
        ss  << "<html><body>" 
            << "<h1>welcome BACK :)</h1>"
            << "<p>Session ID: " << session_id << "</p>"
            << "<p>Visits NO: " << _sessions[session_id] << "</p>"
            << "</body></html>";
        
        res.body = ss.str();
    }
    return res;
}