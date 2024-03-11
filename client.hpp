#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <unistd.h>
#include <sys/socket.h>
#include <set>
#include "channel.hpp"

class Channel;

class Client {
    private :
        int         socket;
        std::string username;
        std::string nickname;
        std::set<std::string>   channels;

    public :
        Client();
        Client(int socket_fd);
        ~Client();

        int get_socket() {return socket;};
        std::string get_username() {return username;};
        std::string get_nickname() {return nickname;};
        int parse_cmd(std::string input, std::map<std::string, Channel*> &channel_list, std::vector<Client *> &client_list);
        int	exec_cmd(std::string input, std::map<std::string, Channel*> &channel_list, std::vector<Client *> &client_list);
        int nick(std::string params);
        int user(std::string params);
        int	topic(std::vector<std::string> params, std::map<std::string, Channel*> &channel_list);
        int join(std::vector<std::string> params, std::map<std::string, Channel*> &channel_list);
        int part(std::vector<std::string> params, std::map<std::string, Channel*> &channel_list);
        int leave_channels(std::map<std::string, Channel*> &channel_list);
        int privmsg(std::vector<std::string> params, std::map<std::string, Channel*> &channel_list);
        int mode(std::vector<std::string> params, std::map<std::string, Channel*> &channel_list);
        int kick(std::vector<std::string> params, std::map<std::string, Channel*> &channel_list);
        int invite(std::vector<std::string> params, std::map<std::string, Channel*> &channel_list, std::vector<Client *> &client_list);
        int sendtochannel(Channel *channel, std::string message);
};

int	    find_client_socket(std::vector<Client *> client_list, Client *client);
Client  *find_client(std::vector<Client *> client_list, std::string client_name);
