#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
#include <vector>
#include <string>

class Transition{
    public:
        std::string state0;
        std::string action;
        std::string state1;
        Transition(std::string state0, std::string action, std::string state1);
        friend std::ostream& operator<<(std::ostream& os, const Transition& t);
        bool operator ==(const Transition& other) const;
};

Transition::Transition(std::string state0, std::string action, std::string state1){
    this->state0 = state0;
    this->action = action;
    this->state1 = state1;
}

std::ostream& operator<<(std::ostream& os, const Transition& t)
{
    os << t.state0 << "--" << t.action << "->" << t.state1;
    return os;
}

bool Transition::operator==(const Transition& other) const{
    if(this->state0.compare(other.state0) == 0 && this->action.compare(other.action) == 0 && this->state1.compare(other.state1) == 0)
        return true;
    else
        return false;
}

class FA{
    public:
        std::vector<std::string> alphabet;
        std::vector<std::string> states;
        std::string start_state;
        std::vector<std::string> final_states;
        std::vector<Transition> transitions;
        FA();
        void read(std::string filename);
        FA* algorithm(FA* nfa);
        void write();
};

void FA::read(std::string filename){
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
        if(element.compare("ALPHABET") == 0){
            state = 0;
            continue;
        }else if(element.compare("STATES") == 0){
            state = 1;
            continue;
        }else if(element.compare("START") == 0){
            state = 2;
            continue;
        }else if(element.compare("FINAL") == 0){
            state = 3;
            continue;
        }else if(element.compare("TRANSITIONS") == 0){
            state = 4;
            continue;
        }
        else if(element.compare("END") == 0){
            continue;
        }

        if(state == 0){
            this->alphabet.push_back(element);
        }else if(state == 1){
            this->states.push_back(element);
        }else if(state == 2){
            this->start_state = element;
        }else if(state == 3){
            this->final_states.push_back(element);
        }else if(state == 4){
            int p_index = 0;
            std::string delimiter = " ";
            int pos = 0;
            std::string token;
            std::string state0;
            std::string action;
            std::string state1;
            while ((pos = element.find(delimiter)) != std::string::npos) {
                token = element.substr(0, pos);
                element.erase(0, pos + delimiter.length());
                if(p_index == 0){
                    state0 = token;
                }else if(p_index = 1){
                    action = token;
                }
                p_index++;
            }
            state1 = element;
            Transition transition(state0, action, state1);
            this->transitions.push_back(transition);
        }
    }  
}

FA::FA(){}

FA* FA::algorithm(FA* nfa){
    std::vector<Transition> new_transitions;
    std::vector<std::string> visit_queue;
    bool found = false;
    std::string start_state = nfa->start_state;
    for(int it = 0; it < nfa->transitions.size(); it++){
        Transition element = nfa->transitions.at(it);
        if(element.state0.compare(start_state) == 0){
            found = false;
            if(new_transitions.empty()){
                new_transitions.push_back(Transition(element.state0, element.action, element.state1));
            }else{
                for(int n_it = 0; n_it < new_transitions.size(); n_it++){
                    if(element.action == new_transitions.at(n_it).action){
                        new_transitions.at(n_it).state1 = new_transitions.at(n_it).state1.append(element.state1);
                        found = true;
                    }
                }
                if(!found){
                    new_transitions.push_back(Transition(element.state0, element.action, element.state1));
                }
            }
        }
    }

    found = false;
    for(int a_it = 0; a_it < nfa->alphabet.size(); a_it++){
        found = false;
        for(int it = 0; it < new_transitions.size(); it++){
            if(new_transitions.at(it).action.compare(nfa->alphabet.at(a_it)) == 0){
                found = true;
            }
        }
        if(!found){
            new_transitions.push_back(Transition(start_state, nfa->alphabet.at(a_it), "S"));
        }
    }
    

    found = false;
    for(int it = 0; it < new_transitions.size(); it++){
        std::string state1 = new_transitions.at(it).state1;
        for(int jit = 0; jit < new_transitions.size(); jit++){
            if(new_transitions.at(jit).state0 == state1){
                found = true;
            }
        }
        if(!found && state1.compare("S") != 0){
            visit_queue.push_back(state1);
        }else{
            found = false;
        }
    }

    int idx = 0;
    std::vector<Transition> visit_transitions;
    while(visit_queue.size() > 0){
        std::string element = visit_queue.at(idx);
        std::vector<char> visit_states(element.begin(), element.end());
        std::string new_start_state = "";
        for(int s_it = 0; s_it < visit_states.size(); s_it++){
            std::string visit_node(1, visit_states.at(s_it));
            new_start_state.append(visit_node);
            for(int t_it = 0; t_it < nfa->transitions.size(); t_it++){
                Transition transition = nfa->transitions.at(t_it);
                if(transition.state0.compare(visit_node) == 0){
                    visit_transitions.push_back(transition);
                }
            }
        }
        
        for(int a_it = 0; a_it < nfa->alphabet.size(); a_it++){
            std::string new_end_state = "";
            std::string alp = nfa->alphabet.at(a_it);
            std::string current_start_state;
            for(int v_it = 0; v_it < visit_transitions.size(); v_it++){
                Transition t = visit_transitions.at(v_it);
                if(new_start_state.find(t.state0) == std::string::npos)
                new_start_state.append(t.state0);
            }
            for(int v_it = 0; v_it < visit_transitions.size(); v_it++){
                Transition t = visit_transitions.at(v_it);
                current_start_state = t.state0;
                if(!t.action.compare(alp)){
                    if(new_end_state.find(t.state1) == std::string::npos)
                    new_end_state.append(t.state1);
                }
            }
            if(new_end_state.compare("") != 0){
                std::sort(new_start_state.begin(), new_start_state.end());
                std::sort(new_end_state.begin(), new_end_state.end());
                Transition new_t(new_start_state, alp, new_end_state);
                bool already_exist = false;
                for(int n_it = 0; n_it < new_transitions.size(); n_it++){
                    Transition t = new_transitions.at(n_it);
                    if(new_t == t){
                        already_exist = true;
                    }
                }
                if(!already_exist){
                    new_transitions.push_back(new_t);
                    visit_queue.push_back(new_t.state1);
                }
            }else{
                Transition new_t(new_start_state, alp, "S");
                bool already_exist = false;
                for(int n_it = 0; n_it < new_transitions.size(); n_it++){
                    Transition t = new_transitions.at(n_it);
                    if(new_t == t){
                        already_exist = true;
                    }
                }
                if(!already_exist){
                    new_transitions.push_back(new_t);
                }
            }
            
        }
        
        visit_states.clear();
        visit_transitions.clear();
        for(int visit_queue_idx = 0; visit_queue_idx < visit_queue.size(); visit_queue_idx++){
            if(visit_queue.at(visit_queue_idx).compare(element) == 0){
                visit_queue.erase(visit_queue.begin() + visit_queue_idx);
            }
        }
        
    }

    for(int it = 0; it < new_transitions.size(); it++){
        if(!new_transitions.at(it).state1.compare("S")){
            for(int a_it = 0; a_it < nfa->alphabet.size(); a_it++){
                new_transitions.push_back(Transition("S", nfa->alphabet.at(a_it), "S"));
            }
            break;
        }
    }
   
    FA* dfa = new FA();

    for(int i = 0; i < nfa->alphabet.size(); i++){
        dfa->alphabet.push_back(nfa->alphabet.at(i));
    }
    dfa->states.push_back(new_transitions.at(0).state0);
    for(int j = 1; j < new_transitions.size(); j++){
        found = false;
        for(int i = 0; i < dfa->states.size(); i++){
            if(new_transitions.at(j).state0.compare(dfa->states.at(i)) == 0)found=true;
        }
        if(!found)dfa->states.push_back(new_transitions.at(j).state0);
    }
    for(int j = 1; j < new_transitions.size(); j++){
        found = false;
        for(int i = 0; i < dfa->states.size(); i++){
            if(new_transitions.at(j).state1.compare(dfa->states.at(i)) == 0)found=true;
        }
        if(!found)dfa->states.push_back(new_transitions.at(j).state0);
    }
    
    dfa->start_state = nfa->start_state;
    
    for(int i = 0; i < new_transitions.size(); i++){
        for(int j = 0; j < nfa->final_states.size(); j++){
            if(new_transitions.at(i).state1.find(nfa->final_states.at(j)) != std::string::npos){
                found = false;
                for(int k = 0; k < dfa->final_states.size(); k++){
                    if(dfa->final_states.at(k).compare(new_transitions.at(i).state1) == 0)found = true;
                }
                if(!found)dfa->final_states.push_back(new_transitions.at(i).state1);
            }
        }
    }
    dfa->transitions = new_transitions;

    return dfa;
}

void FA::write(){

    std::ofstream out_file("DFA.txt");
    out_file << "ALPHABET" << std::endl;
    for(int i = 0; i < this->alphabet.size(); i++){
        out_file << this->alphabet.at(i) << std::endl;
    }
    out_file << "STATES" << std::endl;
    for(int i = 0; i < this->states.size(); i++){
        out_file << this->states.at(i) << std::endl;
    }
    out_file << "START" << std::endl;
    out_file << this->start_state << std::endl;
    out_file << "FINAL" << std::endl;
    for(int i = 0; i < this->final_states.size(); i++){
        out_file << this->final_states.at(i) << std::endl;
    }
    out_file << "TRANSITIONS" << std::endl;
    for(int i = 0; i < this->transitions.size(); i++){
        Transition t = this->transitions.at(i);
        out_file <<  t.state0 << " " << t.action << " " << t.state1 << std::endl;
    }
    out_file << "END";



    std::cout << "ALPHABET" << std::endl;
    for(int i = 0; i < this->alphabet.size(); i++){
        std::cout << this->alphabet.at(i) << std::endl;
    }
    std::cout << "STATES" << std::endl;
    for(int i = 0; i < this->states.size(); i++){
        std::cout << this->states.at(i) << std::endl;
    }
    std::cout << "START" << std::endl;
    std::cout << this->start_state << std::endl;
    std::cout << "FINAL" << std::endl;
    for(int i = 0; i < this->final_states.size(); i++){
        std::cout << this->final_states.at(i) << std::endl;
    }
    std::cout << "TRANSITIONS" << std::endl;
    for(int i = 0; i < this->transitions.size(); i++){
        Transition t = this->transitions.at(i);
        std::cout <<  t.state0 << " " << t.action << " " << t.state1 << std::endl;
    }
    std::cout << "END" << std::endl;


    out_file.close();
}

int main(int argc, char *argv[]){  
    FA* nfa = new FA();
    nfa->read("NFA1.txt");
    FA* dfa = nfa->algorithm(nfa);
    dfa->write();
}
