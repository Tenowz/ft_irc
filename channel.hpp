#pragma once

#include <map>
#include "client.hpp"

class Client;

class Channel {
    private :
        std::string name;
        std::string key;
        std::string topic_name;
        Client  *admin;
        std::vector<Client*>   op_list;
        std::vector<Client*>   client_list;
        std::vector<Client*>   invit_list;
        bool    i;
        bool    t;

    public :
        Channel();
        Channel(Client *admin, std::string name, std::string key);
        ~Channel();

        std::string get_name() {return name;};
        std::string get_key() {return key;};
        std::string getTopic() {return topic_name;};
        std::vector<Client*> get_client_list() {return client_list;};
        Client  *find_client(std::string client_name);
        bool    is_op(Client *client);
        void    add_client(Client *client);
        void    remove_client(Client *client);
        void    invite_only(bool state);
        void    setTopic(std::string newTopic) {this->topic_name = newTopic;};
};