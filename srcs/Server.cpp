/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbari <mbari@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/08 23:36:07 by mbari             #+#    #+#             */
/*   Updated: 2022/05/17 16:55:18 by mbari            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../headers/Server.hpp"


Server::Server() : _name(), _password(), _socketfd(0), _clients(), _pfds(nullptr), _online_c(0), _max_online_c(0), _prefix(":"), _allChannels(), _unavailableUserName(), _clientNicknames() {}

Server::Server(std::string Name, int max_online, std::string Port, std::string Password): _clients()
{
	this->_name = Name;
	this->_max_online_c = max_online + 1;
	this->_password = Password;
	this->_online_c = 0;
	this->_pfds = new struct pollfd[max_online];
	_getSocket(Port);
	this->_pfds[0].fd = this->_socketfd;
	this->_pfds[0].events = POLLIN;
	this->_online_c++;
}

std::string Server::_executeCommand(std::string command, int i)
{
	FILE* pipe = popen(command.c_str(), "r");
	if (!pipe)
		return _printMessage("999", this->_clients[i]->getNickName(), ":Command execution failed");
		
	char buffer[128];
	std::string result = "";
	
	while (!feof(pipe)) {
		if (fgets(buffer, 128, pipe) != NULL)
			result += buffer;
	}
	
	pclose(pipe);
	return _printMessage("200", this->_clients[i]->getNickName(), ":" + result);
}

Server::~Server() 
{
	if (this->_pfds)
		delete [] this->_pfds;
	std::map<int, Client *>::iterator it = this->_clients.begin();
	while (it != this->_clients.end())
	{
		delete it->second;
		it++;
	}
	this->_clients.clear();
	std::map<std::string, Channel *>::iterator itC = this->_allChannels.begin();
	while (itC != this->_allChannels.end())
	{
		delete itC->second;
		itC++;
	}
	this->_allChannels.clear();
}

std::string	Server::_printMessage(std::string num, std::string nickname, std::string message)
{
	if (nickname.empty())
		nickname = "*";
	return (":" + this->_name + " " + num + " " + nickname + " " + message + "\n");
}

void	Server::_newClient(void)
{
	struct sockaddr_storage	remotaddr;
	socklen_t				addrlen;
	int newfd;

	addrlen = sizeof remotaddr;
	newfd = accept(this->_socketfd, (struct sockaddr*)&remotaddr, &addrlen);
	if (newfd == -1)
		std::cout << "accept() error: " << strerror(errno) << std::endl;
	else
	{
		_addToPoll(newfd);
		std::string welcome = _welcomemsg();
		if (send(newfd, welcome.c_str(), welcome.length(), 0) == -1)
			std::cout << "send() error: " << strerror(errno) << std::endl;
		std::cout << "[" << currentDateTime() << "]: new connection from "
			<< inet_ntoa(((struct sockaddr_in*)&remotaddr)->sin_addr)
			<< " on socket " << newfd << std::endl;
	}
}

void Server::startServer(void)
{
	while (77)
	{
		int poll_count = poll(this->_pfds, this->_online_c, -1);
		if (poll_count == -1)
		{
			std::cout << "poll() error: " << strerror(errno) << std::endl;
			exit(-1);
		}

		for (int i = 0; i < this->_online_c; i++)
		{
			if (this->_pfds[i].revents & POLLIN)
			{
				if (this->_pfds[i].fd == this->_socketfd)
					_newClient();			// If listener is ready to read, handle new connection
				else
					_ClientRequest(i);		// If not the listener, we're just a regular client
			}
		}
	}
}

std::string				Server::_getPassword() const { return (this->_password); }
