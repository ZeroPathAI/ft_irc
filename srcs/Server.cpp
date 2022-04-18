/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbari <mbari@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/04/08 23:36:07 by mbari             #+#    #+#             */
/*   Updated: 2022/04/18 23:49:16 by mbari            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../headers/Server.hpp"


Server::Server() : _name(), _unavailableUserName(), _socketfd(0), _pfds(nullptr), _online_c(0), _max_online_c(0), _prefix(":") {};
Server::Server(std::string Name, int max_online, std::string Port)
{
	this->_name = Name;
	this->_max_online_c = max_online + 1;
	this->_online_c = 0;
	this->_pfds = new struct pollfd[max_online];
	this->_clients = new Client[max_online];
	_getSocket(Port);
	this->_pfds[0].fd = this->_socketfd;
	this->_pfds[0].events = POLLIN;
	this->_online_c++;
};

Server::~Server() {}

void	Server::_addToPoll(int newfd)
{
	if (this->_online_c == this->_max_online_c)
	{
		this->_max_online_c *= 2;
		this->_pfds = (struct pollfd *)realloc(this->_pfds, this->_max_online_c);
		this->_clients = (Client *)realloc(this->_clients, this->_max_online_c);
	}
	this->_pfds[this->_online_c].fd = newfd;
	this->_pfds[this->_online_c].events = POLLIN;
	this->_clients[this->_online_c].setClientfd(newfd);

	this->_online_c++;
}

void	Server::_removeFromPoll(int i)
{
	this->_pfds[i] = this->_pfds[this->_online_c - 1];
	// this->_clients[i] = nullptr;

	this->_online_c--;
};

std::string	Server::_welcomemsg(void)
{
	std::string welcome = RED;
	// welcome.append(this->_name);
	welcome.append("██╗    ██╗███████╗██╗      ██████╗ ██████╗ ███╗   ███╗███████╗\n");
	welcome.append("██║    ██║██╔════╝██║     ██╔════╝██╔═══██╗████╗ ████║██╔════╝\n");
	welcome.append("██║ █╗ ██║█████╗  ██║     ██║     ██║   ██║██╔████╔██║█████╗\n");
	welcome.append("██║███╗██║██╔══╝  ██║     ██║     ██║   ██║██║╚██╔╝██║██╔══╝\n");
	welcome.append("╚███╔███╔╝███████╗███████╗╚██████╗╚██████╔╝██║ ╚═╝ ██║███████╗\n");
	welcome.append(" ╚══╝╚══╝ ╚══════╝╚══════╝ ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚══════╝\n");
	welcome.append(BLUE);
	welcome.append("You need to login so you can start chatting OR you can send HELP to see how :) \n");
	welcome.append(RESET);
	return (welcome);
};

std::string	Server::_printError(int num, std::string reply, std::string message)
{
	// std::string ret = num + " " + reply + "\n\t" + message + "\n";
	return (std::to_string(num) + " " + reply + "\n\t" + message + "\n");
	// std::cout << num << " " << reply << "\n\t" << message << std::endl;
}

std::string	Server::_printReply(int num, std::string reply, std::string message)
{
	return (std::to_string(num) + " " + reply + "\n\t" + message + "\n");
}

void	Server::_newClient(void)
{
	/* all those varibles will be deleted when adding client class */
	struct sockaddr_storage	remotaddr;
	socklen_t				addrlen;
	int newfd;
	/* *********************************************************** */

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
		std::cout << "server: new connection from "
			<< inet_ntoa(((struct sockaddr_in*)&remotaddr)->sin_addr)
			<< " on socket " << newfd << std::endl;
	}
};

void	Server::_broadcastmsg(int sender_fd, std::string buf, int nbytes)
{
	for (int j = 0; j < this->_online_c; j++)
	{
		int dest_fd = this->_clients[j].getClientfd();
		// Except the listener and ourselves
		if (dest_fd != this->_socketfd && dest_fd != sender_fd && this->_clients[j].getRegistered())
			if (_sendall(dest_fd, buf) == -1)
				std::cout << "_sendall() error: " << strerror(errno) << std::endl;
	}
}

Request	Server::_splitRequest(std::string req)
{
	// std::vector<std::string>	split;
	// int space = message.find(" ");

	// split.push_back(message.substr(0, space));
	// std::cout << "cmd = " << split[0] << "|"<< std::endl;
	// if (space != std::string::npos)
	// 	split.push_back(message.substr(space + 1, message.length() - 1));
	// std::cout << "args = " << split[1] << std::endl;


	// return (split);
	Request	request;
	size_t	i = 0;
	size_t	j = 0;

	if (req[i] == ' ' || !req[i]) {
		request.invalidMessage = true;
		return (request);
	}
	j = i;
	while (req[i])
	{
		if (req[i] == ' ')
		{
			if (req[i + 1] == ' ') {
				request.invalidMessage = true;
				return (request);
			}
			request.args.push_back(req.substr(j, i - j));
			while (req[i] == ' ')
				i++;
			j = i;
		}
		if (req[i] == ':')
		{
			request.args.push_back(req.substr(i + 1, req.length() - i));
			request.command = request.args[0];
			request.args.erase(request.args.begin());
			return (request);
		}
		i++;
	}

	if (i && req[j])
		request.args.push_back(req.substr(j, i - j));
	request.command = request.args[0];
	request.args.erase(request.args.begin());
	return (request);
}

// std::string	Server::_setUsername( std::string username, int i )
// {
// 	if (username.empty())
// 		return("Username error: USERNAME (your_username)\n");
// 	else if (isdigit(username[0]))
// 		return ("Uesrname can't start with number\n");
// 	else
// 	{
// 		this->_clients[i].setUserName(username);
// 		this->_clients[i].setClientfd(this->_pfds[i].fd);
// 		this->_clients[i].setRegistered(1);
// 		return ("Username set\n");
// 	};
// };

std::string	Server::_sendMessage(std::string message, int i)
{
	if (this->_clients[i].getRegistered())
	{
		std::string	send = this->_clients[i].getUserName() + ": " + message + "\n";
		std::cout << send << std::endl;
		_broadcastmsg(this->_clients[i].getClientfd(), send, send.length());
		return (std::string());
	}
	else
		return("To send messages you Need to Loggin\n");
}

int			Server::_sendall(int destfd, std::string message)
{
	int total = 0;
	int bytesleft = message.length();
	int b;

	while (total < message.length())
	{
		b = send(destfd, message.c_str() + total, bytesleft, 0);
		if (b == -1) break;
		total += b;
		bytesleft -= b;
	}

	return (b == -1 ? -1 : 0);
};

void	Server::_ClientRequest(int i)
{
	/* all those varibles will be deleted when adding client class */
	char buf[6000];
	/* *********************************************************** */

	int sender_fd = this->_pfds[i].fd;
	int nbytes = recv(sender_fd, buf, sizeof(buf), 0);

	std::string message(buf, strlen(buf) - 1);
	std::cout << "message length: " << message.length() << std::endl << "message: " << message << std::endl;
	if (nbytes <= 0)
	{
		if (nbytes == 0)
			std::cout << "server: socket " << sender_fd << " hung up" << std::endl;
		else
			std::cout << "recv() error: " << strerror(errno) << std::endl;

		close(sender_fd);
		_removeFromPoll(i);
	}
	else
	{
		std::string ret = _parsing(message, i);
		if (send(sender_fd, ret.c_str(), ret.length(), 0) == -1)
			std::cout << "send() error: " << strerror(errno) << std::endl;
		// _broadcastmsg( sender_fd, buf, nbytes );	// Send to everyone!
	}
	memset(&buf, 0, 6000);
};

void		Server::_getSocket(std::string Port)
{
	// int socketfd;
	int yes = 1;
	int status;

	struct addrinfo hint, *serverinfo, *tmp;

	memset(&hint, 0, sizeof(hint));
	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_protocol = getprotobyname("TCP")->p_proto;

	status = getaddrinfo(NULL, Port.c_str(), &hint, &serverinfo);

	if (status != 0)
	{
		std::cout << "getaddrinfo() error: " << gai_strerror(status) << std::endl;
		exit(-1);
	}
	for (tmp = serverinfo; tmp != NULL; tmp = tmp->ai_next)
	{
		this->_socketfd = socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_protocol);
		if (this->_socketfd < 0)
			continue;

		setsockopt(this->_socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

		if (bind(this->_socketfd, tmp->ai_addr, tmp->ai_addrlen) < 0)
		{
			close(this->_socketfd);
			continue;
		}
		break;
	}
	freeaddrinfo(serverinfo);

	if (tmp == NULL)
	{
		std::cout << "bind() error: " << strerror(errno) << std::endl;
		exit(-1);
	}

	if (listen(this->_socketfd, this->_max_online_c) == -1)
	{
		std::cout << "listen() error: " << strerror(errno) << std::endl;
		exit(-1);
	}
};

void Server::startServer(void)
{
	struct sockaddr_storage	remotaddr;
	socklen_t				addrlen;
	int newfd;
	char buf[7777];

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
};
