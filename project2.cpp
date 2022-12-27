#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
#include <vector>
#include <string>
#include <map>

class Rule{
    public:
        std::string non_terminal;
        std::string terminal;
        Rule(std::string non_terminal, std::string terminal);
        friend std::ostream& operator<<(std::ostream& os, const Rule& t);
        int getSize();
        bool operator ==(const Rule& other) const;
        bool operator <(const Rule& other) const;
        bool checkIfUnit();
};

Rule::Rule(std::string non_terminal, std::string terminal){
    this->non_terminal = non_terminal;
    this->terminal = terminal;
}

std::ostream& operator<<(std::ostream& os, const Rule& t)
{
    os << t.non_terminal << ":" << t.terminal;
    return os;
}

bool Rule::operator==(const Rule& other) const{
    if(this->non_terminal.compare(other.non_terminal) == 0 && this->terminal.compare(other.terminal) == 0)
        return true;
    else
        return false;
}

bool Rule::operator<(const Rule& other) const{
    return this->non_terminal.compare(other.non_terminal) > 0;
}

bool Rule::checkIfUnit(){
    return this->non_terminal.length() == 1 && this->terminal.length() == 1;
}

class CFG{
    public:
        std::vector<std::string> non_terminals;
        std::vector<std::string> terminals;
        std::vector<Rule> rules;
        std::string start;
        CFG();
        void read(std::string filename);
        void algorithm();
        int getSize();
};

CFG::CFG(){ }

void CFG::read(std::string filename){
    std::vector<std::string> elements;

    std::ifstream file; 
    file.open(filename);   
    std::string line;
    while(getline(file, line))
    {
        elements.push_back(line);
    }
    
    const char* trim_s = " \t\n\r\f\v";
    int state = -1;
    for (int it = 0 ; it < elements.size(); it++){
        std::string element = elements.at(it);
        element = element.erase(element.find_last_not_of(trim_s) + 1);
        if(element.compare("NON-TERMINAL") == 0){
            state = 0;
            continue;
        }else if(element.compare("TERMINAL") == 0){
            state = 1;
            continue;
        }else if(element.compare("RULES") == 0){
            state = 2;
            continue;
        }else if(element.compare("START") == 0){
            state = 3;
            continue;
        }

        if(state == 0){
            this->non_terminals.push_back(element);
        }else if(state == 1){
            this->terminals.push_back(element);
        }else if(state == 2){
            int p_index = 0;
            std::string delimiter = ":";
            int pos = 0;
            std::string token;
            std::string non_terminal;
            std::string terminal;
            while ((pos = element.find(delimiter)) != std::string::npos) {
                token = element.substr(0, pos);
                element.erase(0, pos + delimiter.length());
                if(p_index == 0){
                    non_terminal = token;
                p_index++;
            }
                terminal = element;
                Rule rule(non_terminal, terminal);
                this->rules.push_back(rule);
            }
        }else if(state == 3){
            this->start = element;
        }
    }  
}

// Unnecessary but keep it for now.
int CFG::getSize(){
    return rules.size();
}

void CFG::algorithm(){
    CFG* chomsky = new CFG(); 

    // Handling start rule
    Rule new_start("S0", this->start);
    chomsky->start = "S0";
    chomsky->rules.push_back(new_start);
    chomsky->non_terminals.push_back("S0");

    for(auto it : this->non_terminals)chomsky->non_terminals.push_back(it);
    for(auto it : this->terminals)chomsky->terminals.push_back(it);


    for(auto rit: this->rules){
        chomsky->rules.push_back(rit);
    }

    // Storing epsilon removals to a vector for further process
    std::vector<std::string> epsilon_removals;
    std::vector<std::string> previously_removed_epsilons;
    for(auto it = 0; it < chomsky->getSize(); it++){
        if(!chomsky->rules.at(it).terminal.compare("e") && chomsky->rules.at(it).non_terminal.compare(this->start)){
            epsilon_removals.push_back(chomsky->rules.at(it).non_terminal);
            previously_removed_epsilons.push_back(chomsky->rules.at(it).non_terminal);
            chomsky->rules.erase(chomsky->rules.begin() + it);
        }
    }

    // REMOVAL OF EPSILONS BELOW
    while(epsilon_removals.size() > 0){
        for(auto it = 1; it < chomsky->getSize(); it++){
            std::string non_terminal = chomsky->rules.at(it).non_terminal;
            std::string terminal = chomsky->rules.at(it).terminal;
            if(terminal.length() == 1){
                if(!epsilon_removals.at(0).compare(terminal)){
                    chomsky->rules.push_back(Rule(chomsky->rules.at(it).non_terminal, "e"));
                }
            }else{
                int occurence = 0;
                for(int sit = 0; sit < terminal.length(); sit++){
                    if(!epsilon_removals.at(0).compare(std::string(1, terminal[sit]))){
                        occurence++;
                    }
                }
                if(occurence == 1){
                    bool found = false;
                    for(int sit = 0; sit < terminal.length(); sit++){
                        if(!epsilon_removals.at(0).compare(std::string(1, terminal[sit]))){
                            found = true;
                        }
                    }
                    if(found){
                        std::string new_rule_terminal = "";
                        for(int sit = 0; sit < terminal.length(); sit++){
                            if(!epsilon_removals.at(0).compare(std::string(1, terminal[sit]))){
                                continue;
                            }else{
                                new_rule_terminal.append(std::string(1, terminal[sit]));
                            }
                        }
                        Rule rule(non_terminal, new_rule_terminal);
                        if(non_terminal.compare(new_rule_terminal) && !(std::find(chomsky->rules.begin(), chomsky->rules.end(), rule) != chomsky->rules.end())){
                            chomsky->rules.push_back(rule);
                        }
                    }
                }else if(occurence > 1){
                    std::vector<int>indexes;
                    std::string new_rule_terminal = terminal;
                    for(int sit = 0; sit < terminal.length(); sit++){
                        if(!epsilon_removals.at(0).compare(std::string(1, terminal[sit]))){
                            if(!(std::find(indexes.begin(), indexes.end(), sit) != indexes.end())){
                                new_rule_terminal = terminal.erase(sit, 1);
                                indexes.push_back(sit);
                                terminal.insert(sit, epsilon_removals.at(0));
                                Rule rule(non_terminal, new_rule_terminal);
                                if(non_terminal.compare(new_rule_terminal) && !(std::find(chomsky->rules.begin(), chomsky->rules.end(), rule) != chomsky->rules.end())){
                                    chomsky->rules.push_back(rule);
                                }
                            }
                        }
                    }
                }
            }
        }
        epsilon_removals.erase(epsilon_removals.begin());
        for(int rit = 0; rit < chomsky->rules.size(); rit++){
           if(!chomsky->rules.at(rit).terminal.compare("e") && chomsky->rules.at(rit).non_terminal.compare(this->start)){
                epsilon_removals.push_back(chomsky->rules.at(rit).non_terminal);
                previously_removed_epsilons.push_back(chomsky->rules.at(rit).non_terminal);
                chomsky->rules.erase(chomsky->rules.begin() + rit);
            }
        }
    }

    std::vector<std::string>to_be_removed_non_terminals;
    std::vector<std::string>to_be_removed_terminals;
    for(int rit = 0; rit < chomsky->rules.size(); rit++){
        for(auto nit : chomsky->non_terminals){
            for(auto tit : chomsky->non_terminals){
                if((!nit.compare(chomsky->rules.at(rit).terminal)&& !tit.compare(chomsky->rules.at(rit).non_terminal))){
                    to_be_removed_non_terminals.push_back(tit);
                    to_be_removed_terminals.push_back(nit);
                    for(int sit = 0; sit < chomsky->rules.size(); sit++){
                        if(!chomsky->rules.at(sit).non_terminal.compare(nit)){
                            chomsky->rules.push_back(Rule(tit, chomsky->rules.at(sit).terminal));
                        }
                    }
                }
            }
        }
    }

    for(auto rit = 0; rit < chomsky->rules.size(); rit++){
        for(auto it = 0; it < to_be_removed_non_terminals.size(); it++){
            Rule rule = chomsky->rules.at(rit);
            if(!rule.non_terminal.compare(to_be_removed_non_terminals.at(it)) && !rule.terminal.compare(to_be_removed_terminals.at(it))){
                chomsky->rules.erase(chomsky->rules.begin() + rit);
            }
        }
    }

    for(auto rit = 0; rit < chomsky->rules.size(); rit++){
        Rule rule1(chomsky->rules.at(rit).non_terminal, chomsky->rules.at(rit).terminal);
        for(auto sit = rit + 1; sit < chomsky->rules.size(); sit++){
            Rule rule2(chomsky->rules.at(sit).non_terminal, chomsky->rules.at(sit).terminal);
            if(rule1 == rule2){
                chomsky->rules.erase(chomsky->rules.begin() + sit);
                rit = 0;
            }
        }
    }

    std::map<std::string, std::string> new_terminals;
    for(int i = 0; i < chomsky->terminals.size(); i++){
        std::string non_terminal = "U";
        non_terminal.append(std::to_string(i));
        chomsky->rules.push_back(Rule(non_terminal, chomsky->terminals.at(i)));
        chomsky->non_terminals.push_back(non_terminal);
        new_terminals.insert(std::pair<std::string, std::string>(chomsky->terminals.at(i), non_terminal));
    }

   

    for(auto i = 0; i < chomsky->terminals.size(); i++){
        std::string terminal = chomsky->terminals.at(i);
        for(auto j = 0; j < chomsky->rules.size(); j++){
            std::string rule_terminal = chomsky->rules.at(j).terminal;
            if(rule_terminal.length() > 1){
                std::string new_rule_terminal;
                for(int k = 0; k < rule_terminal.length(); k++){
                    std::string splitted_terminal(1, rule_terminal[k]);
                    if(!terminal.compare(splitted_terminal)){
                        new_rule_terminal.append(new_terminals[terminal]);

                    }else{
                        new_rule_terminal.append(std::string(splitted_terminal));
                    }
                }
                chomsky->rules.at(j).terminal = new_rule_terminal;  

            }
        }
    }

    

    std::vector<std::string> terminals_to_be_shortened;
    for(int i = 0; i < chomsky->rules.size(); i++){
        std::string terminal = chomsky->rules.at(i).terminal;
        int terminal_length = 0;
        for(int j = 0; j < terminal.length(); j++){
            terminal_length++;
            if(terminal[j] == 'U'){
                j++;
            }
        }
        if(terminal_length > 2){
            if(!(std::find(terminals_to_be_shortened.begin(), terminals_to_be_shortened.end(), terminal) != terminals_to_be_shortened.end())){
                terminals_to_be_shortened.push_back(terminal);
            }
        }
    }

   

    int index = 0;
    int t_index = 0;
    std::map<std::string, std::string> new_terminal_map;
    for(auto i = 0; i < terminals_to_be_shortened.size(); i++){
        std::string terminal = terminals_to_be_shortened.at(i);
        int terminal_length = 0;
        for(int j = 0; j < terminal.length(); j++){
            terminal_length++;
            if(terminal[j] == 'U'){
                j++;
            }
        }
        int divider = terminal_length / 2;
        
        t_index = 0;
        while(t_index < divider){
            std::string shortened_terminal;
            std::string non_terminal = "Z";
            non_terminal.append(std::to_string(index++));
            std::string new_short_terminal;
            
            if(terminal[t_index] == 'U' && terminal[t_index + 2] != 'U'){
                new_short_terminal.append(terminal.substr(t_index, t_index + 3));
                shortened_terminal.append(non_terminal);
                t_index += 3;
            }else if(terminal[t_index] == 'U' && terminal[t_index + 2] == 'U'){
                new_short_terminal.append(terminal.substr(t_index, t_index + 4));
                shortened_terminal.append(non_terminal);
                t_index += 4;
            }else if(terminal[t_index] != 'U' && terminal[t_index + 1] == 'U'){
                new_short_terminal.append(terminal.substr(t_index, t_index + 3));
                shortened_terminal.append(non_terminal);
                t_index += 3;
            }else{
                new_short_terminal.append(terminal.substr(t_index, t_index + 2));
                shortened_terminal.append(non_terminal);
                t_index += 2;
            }
            for(int k = t_index; k < terminal.length(); k++){
                shortened_terminal.append(std::string(1, terminal[k]));
            }
            chomsky->rules.push_back(Rule(non_terminal, new_short_terminal));
            new_terminal_map.insert(std::pair<std::string, std::string>(new_short_terminal, non_terminal));

        }
    }

   
    for(auto i = 0; i < chomsky->rules.size(); i++){
        Rule rule1 = chomsky->rules.at(i);
        for(auto j = 0; j < chomsky->rules.size(); j++){
            Rule rule2 = chomsky->rules.at(j);
            if(rule1.non_terminal[0] == 'Z' && rule2.non_terminal[0] == 'Z'){
                if(!rule1.terminal.compare(rule2.terminal) && rule1.non_terminal.compare(rule2.non_terminal))
                    chomsky->rules.erase(chomsky->rules.begin() + j);
            }
        }
    }

    for(auto elem : new_terminal_map){
        for(auto it = 0; it < chomsky->rules.size(); it++){
            if(chomsky->rules.at(it).terminal.find(elem.first) != std::string::npos ){
                int start_index = chomsky->rules.at(it).terminal.find(elem.first);
                if(chomsky->rules.at(it).non_terminal[0] != 'Z'){
                    chomsky->rules.at(it).terminal.replace(start_index, elem.first.length(), elem.second);
                }
            }
        }
    }

    for(auto it : chomsky->rules){
        if(it.non_terminal[0] == 'U' || it.non_terminal[0] == 'Z'){
            if(!(std::find(chomsky->non_terminals.begin(), chomsky->non_terminals.end(), it.non_terminal) != chomsky->non_terminals.end()))
            chomsky->non_terminals.push_back(it.non_terminal);
        }
    }

    std::cout << "NON-TERMINALS" << std::endl;
    for(int i = 0; i < chomsky->non_terminals.size(); i++){
        std::cout << chomsky->non_terminals.at(i) << std::endl;
    }

    std::cout << "TERMINALS" << std::endl;
    for(int i = 0; i < chomsky->terminals.size(); i++){
        std::cout << chomsky->terminals.at(i) << std::endl;
    }

    std::cout << "RULES" << std::endl;
    for(int i = 0; i < chomsky->rules.size(); i++){
        std::cout << chomsky->rules.at(i) << std::endl;
    }
    std::cout << "START" << std::endl << chomsky->start << std::endl;
}

int main(int argc, char *argv[]){  
    CFG* cfg = new CFG();
    cfg->read("G1.txt");
    cfg->algorithm();

    return 0;
}