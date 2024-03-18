#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstddef>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <errno.h>
#include <sys/select.h>
#include <string>
#include <vector>
#include <map>
#include <csignal>
#include "client.hpp"

#define BUFFER_SIZE 1000

int init_serv_socket()
{
	const int	serv_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (serv_socket == -1) {
		perror("socket");
		exit(-1);
	}
	const int trueFlag = 1;
	if (setsockopt(serv_socket, SOL_SOCKET, SO_REUSEADDR, &trueFlag, sizeof(int)) < 0){
		perror("setsockopt");
		exit(-1);
	}
	return (serv_socket);
}

void	show_caracters(std::string str, std::string clientName)
{
	std::cout << clientName + " : ";
	for (size_t i = 0; i < str.size(); i++) {
        if (isprint(str[i])) {
            std::cout << str[i];
        } else {
			switch(str[i]) {
                case '\n':
                    std::cout << "\\n";
                    break;
                case '\r':
                    std::cout << "\\r";
                    break;
                case '\t':
                    std::cout << "\\t";
                    break;
                default:
                    std::cout << static_cast<int>(str[i]);
                    break;
			}
        }
    }
	std::cout << std::endl;
}

int main(int argc, char **argv)
{
	struct sockaddr_in				addr;
	std::string						buff;
	std::string						user;
	std::string						nick;
	std::string						pass_msg;
	fd_set							read_fds;
	socklen_t						addr_len;
	Client							*newClient;
	std::vector<Client*> 			client_list;
	std::map<std::string, Channel*>	channel_list;
	char							buffer[BUFFER_SIZE];
	int								new_client_socket, fd_max, read_value;

	if (argc != 3 || atoi(argv[1]) < 1024 || atoi(argv[1]) > 65353) {

		std::cout << "Wrong arguments !" << std::endl;
		return 1;
	}
	const int	serv_socket = init_serv_socket();
	addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(atoi(argv[1]));
	if (bind(serv_socket, (struct sockaddr *)&addr, sizeof(addr)))
		return (errno);
	addr_len = sizeof(addr);
	if (listen(serv_socket, 10))
		return (3);
	std::cout << "IRC server launched !" << std::endl << "Waiting for clients..." << std::endl;
	while (1) {

		FD_ZERO(&read_fds);
		FD_SET(serv_socket, &read_fds);
		fd_max = serv_socket;
		for (size_t i = 0; i < client_list.size(); i++) {

			if (client_list[i]->get_socket() > 0)
				FD_SET(client_list[i]->get_socket(), &read_fds);
			if (client_list[i]->get_socket() > fd_max)
				fd_max = client_list[i]->get_socket();
		}
		select(fd_max + 1 , &read_fds , NULL , NULL , NULL);
		if (FD_ISSET(serv_socket, &read_fds)) {

			if ((new_client_socket = accept(serv_socket, (struct sockaddr *)&addr, &addr_len)) < 0) {
				perror("accept");
				return (-1);
			}
			std::cout << "New connection attempt from IP " << inet_ntoa(addr.sin_addr) << " and PORT N°" << ntohs(addr.sin_port) << std::endl;
			buff.clear();
			pass_msg = "Enter the server password : ";
			send(new_client_socket, pass_msg.c_str(), pass_msg.size(), 0);
			while (1) {

				read_value = read(new_client_socket, buffer, BUFFER_SIZE);
				buffer[read_value] = '\0';
				buff += buffer;
				if (buff.find('\r') == std::string::npos && buff.find('\n') == std::string::npos)
					continue;
				buff.erase(buff.find('\n'), 1);
				if (read_value > 0 && strcmp(buff.c_str(), argv[2]) == 0)
					break ;
				else {

					std::cout << "Connection attempt failed" << std::endl;
					pass_msg = "Wrong password !\nPlease try again : ";
					send(new_client_socket, pass_msg.c_str(), pass_msg.size(), 0);
					buff.clear();
				}
			}
			buff.clear();
			pass_msg = "Enter a username : ";
			send(new_client_socket, pass_msg.c_str(), pass_msg.size(), 0);
			while (1) {

				read_value = read(new_client_socket, buffer, BUFFER_SIZE);
				buffer[read_value] = '\0';
				buff += buffer;
				if (buff.find('\r') == std::string::npos && buff.find('\n') == std::string::npos)
					continue;
				buff.erase(buff.find('\n'), 1);
				user = buff;
				if (read_value > 0 && user.find_first_of(" \t\n\v\f\r") == std::string::npos && user.empty() == false && user[0] && !findUsername(client_list, user))
					break ;
				else {

					pass_msg = "Invalid username !\nPlease try again : ";
					send(new_client_socket, pass_msg.c_str(), pass_msg.size(), 0);
					buff.clear();
				}
			}
			buff.clear();
			pass_msg = "Enter a nickname : ";
			send(new_client_socket, pass_msg.c_str(), pass_msg.size(), 0);
			while (1) {

				read_value = read(new_client_socket, buffer, BUFFER_SIZE);
				buffer[read_value] = '\0';
				buff += buffer;
				if (buff.find('\r') == std::string::npos && buff.find('\n') == std::string::npos)
					continue;
				buff.erase(buff.find('\n'), 1);
				nick = buff;
				if (read_value > 0 && nick.find_first_of(" \t\n\v\f\r") == std::string::npos && nick.empty() == false && nick[0] && !findNickname(client_list, nick))
					break ;
				else {

					pass_msg = "Invalid nickname !\nPlease try again : ";
					send(new_client_socket, pass_msg.c_str(), pass_msg.size(), 0);
					buff.clear();
				}
			}
			std::cout << "Client successfully joined the server" << std::endl;
			newClient = new Client(new_client_socket, user, nick);
			std::cout << "New client username : " << newClient->get_username() << "\nNew client nickname : " << newClient->get_nickname() << std::endl;
			client_list.push_back(newClient);
			pass_msg = "You have successfully joined the server !\n";
			send(new_client_socket, pass_msg.c_str(), pass_msg.size(), 0);
		}
		for (std::vector<Client *>::iterator it = client_list.begin(); it < client_list.end(); it++) {

			buff.clear();
			if (FD_ISSET((*it)->get_socket(), &read_fds)) {

				while (1) {
					
					read_value = read((*it)->get_socket(), buffer, BUFFER_SIZE);
					buffer[read_value] = '\0';
					buff += buffer;
					if (buff.find('\r') == std::string::npos && buff.find('\n') == std::string::npos)
						continue;
					else
						break ;
				}
				buff.erase(buff.find('\n'), 1);
				if (read_value < 0)
					std::cout << "read error" << std::endl;
				else if (read_value == 0) {

					getpeername((*it)->get_socket(), (struct sockaddr *)&addr, &addr_len);
					std::cout << "Connection lost from IP " << inet_ntoa(addr.sin_addr) << " and PORT N°" << ntohs(addr.sin_port) << std::endl;
					FD_CLR((*it)->get_socket(), &read_fds);
					(*it)->leave_channels(channel_list);
					it = client_list.erase(it);
					close((*it)->get_socket());
					delete *it;
				}
				else {

					(*it)->parse_cmd(buff, channel_list, client_list);
					if (strncmp(buff.c_str(), "QUIT", 5) == 0)
					{
						std::cout << "Client from IP " << inet_ntoa(addr.sin_addr) << " and PORT N°" << ntohs(addr.sin_port) << " has been removed" << std::endl;
						FD_CLR((*it)->get_socket(), &read_fds);
						(*it)->leave_channels(channel_list);
						it = client_list.erase(it);
						close((*it)->get_socket());
						delete *it;
					}
				}
			}
		}
	}
	return (0);
}
