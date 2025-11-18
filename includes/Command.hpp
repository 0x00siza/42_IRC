#pragma once

class Server;
class Client;

class Command{
    public:
        string prefix;
        string command;
        vector<string> params;

        Command() {}

};