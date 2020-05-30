#include <set>
#include <queue>
#include <iostream>
#include "bfs.h"

// Naimplementujte efektivni algoritmus pro nalezeni nejkratsi cesty v grafu.
// V teto metode nemusite prilis optimalizovat pametove naroky, a vhodnym algo-
// ritmem tak muze byt napriklad pouziti prohledavani do sirky (breadth-first
// search.
//
// Metoda ma za ukol vratit ukazatel na cilovy stav, ktery je dosazitelny pomoci
// nejkratsi cesty. Pokud je nejkratsich cest vice, vratte ukazatel na stav s nej-
// nizsim identifikatorem (viz metoda 'state::get_identifier()' v 'state.h').

std::shared_ptr<const state> bfs(std::shared_ptr<const state> root) {
    std::shared_ptr<const state> result = NULL;
    // IDs of already visited states
    std::set<unsigned long long> visited;
    std::queue<std::shared_ptr<const state>> queue;
    queue.push(root);
    visited.insert(root->get_identifier());
    while(!queue.empty()) {
        unsigned size = queue.size();
        #pragma omp parallel for
        for (unsigned i = 0; i < size; i++) {
            std::shared_ptr<const state> curState;
            #pragma omp critical
            {
                curState = queue.front();
                queue.pop();
            }
            // process neighbours
            for (std::shared_ptr<const state> nextState : curState->next_states()) {
                unsigned long long stateID = nextState->get_identifier();
                // if state wasn't visited yet
                if (visited.find(stateID) == visited.end()) {
                    bool isGoal = nextState->is_goal();
                    if (isGoal) {
                        if (result == NULL) {
                            #pragma omp critical
                            result = nextState;
                        }
                        else if (result->get_identifier() > nextState->get_identifier()) {
                            #pragma omp critical
                            {
                                result = nextState;
                                visited.insert(stateID);
                            }
                        }
                    }
                    else {
                        #pragma omp critical
                        {
                            queue.push(nextState);
                            visited.insert(stateID);
                        }
                    }
                }
            }
        }
        if (result != NULL) break;
    }
    return result;
}