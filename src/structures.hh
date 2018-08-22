#pragma once

#include <vector>
#include <tuple>
#include <deque>

using graph_ty = std::vector <std::vector<bool>>;

// tuple<terrain, arbitre, team1, team2>
using match_ty = std::tuple<int, int, int, int>;
using match_list = std::vector<match_ty>;

using teams_ty = std::tuple<int, int, int>;

// Class representing a team
struct Team {
    // Enum of actions a team can take during one round
    // Only one action per round
    enum state {
        PLAY,
        SLEEP,
        JUDGE
    };

    Team(int n)
            : id{n} {}

    // The team unique identifier
    int id;
    // The list of action taken by the team
    std::deque <state> actions;

    // Count the number of 'T' action taken by the team
    template<state T>
    inline int count() const {
        auto res = 0;
        for (const auto &elm : actions)
            res += elm == T;
        return res;
    }

    // Add an action to the list
    inline void push(state t) {
        actions.push_back(t);
    }
};

// Type of Team placeholder
using teams_list = std::vector<Team>;

// temporary Type representing a match (when the field is not decided yet)
using match_tup = std::vector <std::tuple<Team *, Team *, Team *>>;
