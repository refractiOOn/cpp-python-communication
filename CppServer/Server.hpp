#pragma once

#include "pch.hpp"
#include "functions.hpp"

void __stdcall WSACleanupDestructor(WSADATA *)
{
    WSACleanup();
}
using unique_wsadata = wil::unique_struct<WSADATA, decltype(WSACleanupDestructor), WSACleanupDestructor>;

class Server
{
public:
    Server(int port) : m_socket(), m_sockAddr()
    {
        if (WSAStartup(MAKEWORD(2, 2), &m_wsaData))
        {
            throw std::runtime_error("WSA startup failed");
        }

        m_socket.reset(socket(AF_INET, SOCK_STREAM, 0));
        if (!m_socket)
        {
            throw std::runtime_error("Server socket creating failed");
        }

        m_sockAddr.sin_family = AF_INET;
        m_sockAddr.sin_port = htons(port);
        m_sockAddr.sin_addr.s_addr = INADDR_ANY;
        if (bind(m_socket.get(), reinterpret_cast<SOCKADDR *>(&m_sockAddr), sizeof(m_sockAddr)))
        {
            throw std::runtime_error("Server socket binding failed: " + WSAGetLastError());
        }
    }

    ~Server() = default;

    void Listen() noexcept
    {
        try
        {
            if (listen(m_socket.get(), SOMAXCONN))
            {
                throw std::runtime_error("Listening failed: " + WSAGetLastError());
            }
            std::cout << "Listening..." << std::endl;

            int addrSize = sizeof(m_sockAddr);
            wil::unique_socket clientSocket;
            while (true)
            {
                clientSocket.reset(accept(m_socket.get(), reinterpret_cast<SOCKADDR *>(&m_sockAddr), &addrSize));
                if (!clientSocket)
                {
                    throw std::runtime_error("Client socket creating failed: " + WSAGetLastError());
                }

                m_clients.emplace_back(std::jthread(&Server::HandleClient, this, std::move(clientSocket)));
            }
        }
        catch(const std::exception& ex)
        {
            std::cerr << ex.what() << std::endl;
        }
    }

private:
    void HandleClient(wil::unique_socket clientSocket) noexcept
    {
        try
        {
            std::cout << "Client accepted" << std::endl;

            std::string buf(1024, '\0');
			std::size_t numBytes = recv(clientSocket.get(), &buf[0], buf.size(), 0);
			if (numBytes <= 0)
			{
				throw std::runtime_error("Client disconnected before data was sent");
			}

			std::cout << "Buffer from client: " << buf.substr(0, numBytes) << std::endl;

			rapidjson::Document doc;
			if (doc.Parse(buf.c_str()).HasParseError())
			{
				throw std::runtime_error("JSON parse error");
			}

			if (!doc.HasMember("operation"))
			{
				std::cout << "No 'operation' member" << std::endl;
				throw std::logic_error("No \"operation\" member");
			}

			std::string op = doc["operation"].GetString(), response;
			if (op == "Plus")
			{
				if (!doc.HasMember("arg1") || !doc.HasMember("arg2"))
				{
					throw std::logic_error("Invalid arguments");
				}

				response = GenerateResponse(Plus(doc["arg1"].GetInt(), doc["arg2"].GetInt()));

				std::cout << "Plus operation handled" << std::endl;
			}
			else if (op == "Minus")
			{
				if (!doc.HasMember("arg1") || !doc.HasMember("arg2"))
				{
					throw std::logic_error("Invalid arguments");
				}

				response = GenerateResponse(Minus(doc["arg1"].GetInt(), doc["arg2"].GetInt()));

				std::cout << "Minus operation handled" << std::endl;
			}
			else if (op == "Multiply")
			{
				if (!doc.HasMember("arg1") || !doc.HasMember("arg2"))
				{
					throw std::logic_error("Invalid arguments");
				}

				response = GenerateResponse(Multiply(doc["arg1"].GetInt(), doc["arg2"].GetInt()));

				std::cout << "Multiply operation handled" << std::endl;
			}
			else if (op == "Divide")
			{
				if (!doc.HasMember("arg1") || !doc.HasMember("arg2"))
				{
					throw std::logic_error("Invalid arguments");
				}

				response = GenerateResponse(Divide(doc["arg1"].GetInt(), doc["arg2"].GetInt()));

				std::cout << "Divide operation handled" << std::endl;
			}
			else if (op == "Concat")
			{
				if (!doc.HasMember("arg1") || !doc.HasMember("arg2"))
				{
					throw std::logic_error("Invalid arguments");
				}

				response = GenerateResponse(Concat(doc["arg1"].GetString(), doc["arg2"].GetString()));

				std::cout << "Concat operation handled" << std::endl;
			}
			else
			{
				throw std::logic_error("Unsupported operation");
			}

			if (send(clientSocket.get(), response.c_str(), response.size(), 0) <= 0)
			{
				throw std::runtime_error("Sending response error");
			}

			std::cout << "Client socket closed" << std::endl;
        }
        catch(const std::exception& ex)
        {
            std::cerr << ex.what() << std::endl;
        }
    }

    template<typename T>
	std::string GenerateResponse(T result)
	{
		rapidjson::StringBuffer response;
		rapidjson::Writer<rapidjson::StringBuffer> responseWriter(response);
		responseWriter.StartObject();
		responseWriter.Key("response");
		SetValueToWriter(responseWriter, result);	

		responseWriter.EndObject();

		return response.GetString();
	}

    void SetValueToWriter(rapidjson::Writer<rapidjson::StringBuffer> &responseWriter, int value)
	{
		responseWriter.Int(value);
	}
	void SetValueToWriter(rapidjson::Writer<rapidjson::StringBuffer> &responseWriter, double value)
	{
		responseWriter.Double(value);
	}
	void SetValueToWriter(rapidjson::Writer<rapidjson::StringBuffer> &responseWriter, const std::string &value)
	{
		responseWriter.String(value.c_str());
	}

private:
    unique_wsadata m_wsaData;
    wil::unique_socket m_socket;
    SOCKADDR_IN m_sockAddr;

    std::vector<std::jthread> m_clients;

};