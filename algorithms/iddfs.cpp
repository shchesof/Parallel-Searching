#include <set>
#include <queue>
#include <iostream>
#include <omp.h>
#include "iddfs.h"

// Naimplementujte efektivni algoritmus pro nalezeni nejkratsi (respektive nej-
// levnejsi) cesty v grafu. V teto metode mate ze ukol naimplementovat pametove
// efektivni algoritmus pro prohledavani velkeho stavoveho prostoru. Pocitejte
// s tim, ze Vami navrzeny algoritmus muze bezet na stroji s omezenym mnozstvim
// pameti (radove nizke stovky megabytu). Vhodnym pristupem tak muze byt napr.
// iterative-deepening depth-first search.
//
// Metoda ma za ukol vratit ukazatel na cilovy stav, ktery je dosazitelny pomoci
// nejkratsi/nejlevnejsi cesty. Pokud je nejkratsich cest vice, vratte ukazatel
// na stav s nejnizsim identifikatorem (viz methoda 'state::get_identifier()').

std::queue<std::shared_ptr<const state>> queue;
std::set<unsigned long long> visited;

// depth-limited search
std::shared_ptr<const state> dls(std::shared_ptr<const state> root, int level) {
    // TODO delete this check
    if (root->is_goal()) {
        return root;
    }
    if(level <= 0) {
        return nullptr;
    }
    std::shared_ptr<const state> tmp;
    std::shared_ptr<const state> result = nullptr;
    for(std::shared_ptr<const state> nextState : root->next_states()) {
        // if was visited in BFS earlier
        if (visited.find(nextState->get_identifier()) != visited.end()) {
            continue;
        }
        if (nextState->is_goal()) {
            if (result == nullptr || result->get_identifier() > nextState->get_identifier()) {
                result = nextState;
                continue; // mehhh
            }
        }
        bool isVisited = false;
        tmp = root;
        while(tmp != nullptr) {
            if (nextState->get_identifier() == tmp->get_identifier()) {
                isVisited = true;
                break;
            }
            tmp = tmp->get_predecessor();
        }
        if(!isVisited) {
            std::shared_ptr<const state> tmpResult = dls(nextState, level - 1);
            if (tmpResult != nullptr) {
                if (result == nullptr || result->get_identifier() > tmpResult->get_identifier()) {
                    result = tmpResult;
                }
            }
        }
    }
    return result;
}

// traverses small part of graph by BFS
std::shared_ptr<const state> tryBFS(std::shared_ptr<const state> root) {
    int threads = omp_get_max_threads();
    queue.push(root);
    std::shared_ptr<const state> result = nullptr;
    visited.insert(root->get_identifier());
    while(true) {
        unsigned size = queue.size();
        for(unsigned i = 0; i < size; i++) {
            std::shared_ptr<const state> curState = queue.front();
            queue.pop();
            for(std::shared_ptr<const state> nextState : curState->next_states()) {
                unsigned long long stateID = nextState->get_identifier();
                if(visited.find(stateID) == visited.end()) {
                    if (nextState->is_goal()) {
                        if (result == nullptr || result->get_identifier() > nextState->get_identifier()) {
                            result = nextState;
                            visited.insert(stateID);
                        }
                    }
                    else {
                        queue.push(nextState);
                        visited.insert(stateID);
                    }
                }
            }
        }
        if(queue.size() > threads || result != nullptr) break;
    }
    return result;
}

std::shared_ptr<const state> iddfs(std::shared_ptr<const state> root) {
    std::shared_ptr<const state> result = tryBFS(root);
    if(result != nullptr) {
        return result;
    }
    int level = 2;
    std::vector<std::shared_ptr<const state>> states;
    // TODO without copying
    // copy queue to vector to process 'roots' states
    while (!queue.empty()) {
        std::shared_ptr<const state> state = queue.front();
        states.push_back(state);
        queue.pop();
    }
    while(true) {
#pragma omp parallel
        {
            std::shared_ptr<const state> tmpResult;
#pragma omp for schedule(dynamic)
            for(int i = 0; i < states.size(); i++) {
                tmpResult = dls(states.at(i), level);
                if(tmpResult != nullptr) {
                    if (result == nullptr || result->get_identifier() > tmpResult->get_identifier()) {
#pragma omp critical
                        {
                            result = tmpResult;
                        }
                    }
                }
            }
        }
        if(result != nullptr) break;
        level++;
    }
    return result;
}