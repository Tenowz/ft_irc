#pragma once

#include <map>
#include "client.hpp"

class Client;

class Channel {
	private :
		int			limit;
		std::string name;
		std::string key;
		std::string topic_name;
		Client  *admin;
		std::vector<Client*>   op_list;
		std::vector<Client*>   client_list;
		std::vector<Client*>   invit_list;
		bool    i;
		bool    t;
		bool    k;
		bool	l;

	public :
		Channel();
		Channel(Client *admin, std::string name, std::string key);
		~Channel();

		bool	getKeyState() {return k;};
		bool	getLimitState() {return l;};
		bool    getInviteState() {return i;};
		bool    getTopicState() {return t;};
		size_t	getLimit() {return limit;};
		std::string get_name() {return name;};
		std::string get_key() {return key;};
		std::string getTopic() {return topic_name;};
		std::vector<Client*> get_client_list() {return client_list;};
		std::vector<Client*> getInviteList() {return invit_list;};
		std::vector<Client*> getOpList() {return op_list;};
		Client  *find_client(std::string client_name);
		Client  *findInvited(int const socket);
		bool    is_op(Client *client);
		void    add_client(Client *client);
		void    remove_client(Client *client);
		void    invite_only(bool state) {this->i = state;};
		void    topicOpOnly(bool state) {this->t = state;};
		void	keyOnly(bool state) {this->k = state;};
		void	limitOnly(bool state) {this->l = state;};
		void	setLimit (std::string limit, int socket);
		void    setTopic(std::string newTopic) {this->topic_name = newTopic;};
};