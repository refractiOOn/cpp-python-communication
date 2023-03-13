#include "pch.hpp"
#include "Server.hpp"

int main(int, char**)
{
    Server server(8080);
    server.Listen();
}