#include "match.hh"

#include <algorithm>
#include <string>

// Check every match has been played
// (the array/graph only contains True)
bool done(const graph_ty &g) {
    for (const auto &elm : g)
        for (auto b : elm)
            if (!b)
                return false;
    return true;
}

// Count the number of match remaining to be played
int count_left(const graph_ty &g, int max) {
    int res = 0;
    for (const auto &elm : g)
        for (auto b : elm)
            if (b) {
                res++;
                if (res == max)
                    return max;
            }
    return res;
}

// List the remaining matches (order: highest priority first)
// return a list of tuple (pair)
// Each tuple represent the two playing teams.
std::vector <std::tuple<Team *, Team *>> list_remaining(const graph_ty &g, teams_list &teams) {
    std::vector <std::tuple<Team *, Team *>> res;

    // list every non played match.
    for (auto i = 0; i < g.size(); ++i)
        for (auto j = 0; j < i; ++j)
            if (!g[i][j])
                res.push_back(std::make_tuple(&teams[j], &teams[i]));

    // Sort the list by priority (least played first, most slept then)
    std::sort(res.begin(), res.end(),
            // comparison function (returns true if tuple A's priority is superior than tuple B's)
              [](const std::tuple<Team *, Team *> &a, const std::tuple<Team *, Team *> &b) {
                  // sum number of match played by both teams until now
                  int sum_a = std::get<0>(a)->count<Team::PLAY>() + std::get<1>(a)->count<Team::PLAY>();
                  int sum_b = std::get<0>(b)->count<Team::PLAY>() + std::get<1>(b)->count<Team::PLAY>();
                  if (sum_a != sum_b)
                      return sum_a < sum_b;
                  // sum number of match spent sleeping by both teams until now
                  sum_a = std::get<0>(a)->count<Team::SLEEP>() + std::get<1>(a)->count<Team::SLEEP>();
                  sum_b = std::get<0>(b)->count<Team::SLEEP>() + std::get<1>(b)->count<Team::SLEEP>();
                  return sum_a > sum_b;
              });
    return res;
};

// Create the array/graph of match to play
graph_ty gen_matches(int nb_team) {
    graph_ty res;
    for (auto i = 0; i < nb_team; i++) {
        res.emplace_back();
        for (auto j = 0; j < nb_team; j++)
            res.back().emplace_back(j == i);
    }
    return res;
}

// Create the placeholder for a given number of teams
teams_list gen_teams(int nb_team) {
    teams_list res;
    for (auto i = 0; i < nb_team; i++)
        res.push_back(Team(i));
    return res;
}

// Clone a team placeholder (clone will then be ordered)
std::vector<Team *> clone(teams_list &teams) {
    std::vector < Team * > res;
    for (auto &elm : teams)
        res.push_back(&elm);

    return res;
}

// Initiate internal data structures and return the generated matches
match_list make_match(int nb_team, int nb_field) {
    // init array/graph
    auto graph = gen_matches(nb_team);

    // const teams placeholder (static ordering)
    teams_list teams = gen_teams(nb_team);
    // clone of 'teams' (will be reordered later)
    std::vector < Team * > teams_cln = clone(teams);
    // result (match list)
    match_list res;

    // round counter
    int nb_round = 0;

    while (!done(graph)) {
        // generate a round
        gen_round(res, graph, teams_cln, teams, nb_field);

        // increment round counter
        nb_round++;
        // add sleep to teams (keep number of action equal for every team)
        apply_sleep(teams, nb_round);
    }

    return res;
}

// Make every team that did not play on rule take turns of sleep.
void apply_sleep(teams_list &teams, int num) {
    for (auto &elm : teams)
        while (elm.actions.size() < num)
            elm.push(Team::SLEEP);
}

// Generate a single round
// bur : result buffer
// graph: array/graph of matches
// teams: team list
// teams_stat (the fixed ordered list mostly equal to teams)
// nb_fields: number of field that can be used
void gen_round(match_list &buf, graph_ty &graph, std::vector<Team *> &teams, teams_list &teams_stat, int nb_field) {

    // TODO: fix the arithmetic
    const int nb_match = graph.size() * (graph.size() - 1) / 2;
    // number of match to aim for this round
    int max = count_left(graph, nb_field);

    // list every remaining match to play (order: highest priority first)
    auto possible = list_remaining(graph, teams_stat);

    // buffer for currently selected matches for the round
    match_tup selected;

    // Standard algorithm + functor
    // Simply: sort the list of teams (order: highest judging priority first)
    std::sort(teams.begin(), teams.end(),
            // Comparison function
            // return true if A has higher priority than B
              [](const Team *a, const Team *b) {

                  int sum_a = a->count<Team::JUDGE>();
                  int sum_b = b->count<Team::JUDGE>();
                  // compare number of time spent judging
                  if (sum_a != sum_b)
                      return sum_a < sum_b;
                  // compare number of time spent sleeping
                  return a->count<Team::SLEEP>() > b->count<Team::SLEEP>();
              });

    // try to find 'max' match to play
    for (auto i = 0; i < max; ++i) {
        bool found = false;

        // look through every possible match (previously listed)
        for (auto it = possible.begin(); it != possible.end(); ++it)
            // check if playing teams are compatible with selected
            if (compatible(selected, std::get<0>(*it)) && compatible(selected, std::get<1>(*it))) {
                // choose a judge for the match
                auto judge = find_judge(selected, *it, teams, nb_match);
                // if no judge can be found just consider the match as not compatible
                if (!judge)
                    // TODO: improvement: maybe list this kind of matches somewhere
                    //                    cause they are the one than could be switched
                    continue;
                found = true;

                // select the match (add it to selected)
                selected.push_back(std::make_tuple<>(judge, std::get<0>(*it), std::get<1>(*it)));

                // remove the selected match from the possible ones (prevent duplicates)
                possible.erase(possible.begin(), it + 1);
                break;
            }

        // stop computation if no match found
        if (!found)
            break;
    }

    // check error
    if (selected.size() == 0)
        throw std::runtime_error("cannot find any valid match to play");

    // apply the selection to buf
    apply_selection(buf, graph, selected);
}

// Add the selected matches as a new round
// update the array/graph of played matches
// make playing and judging team take there action
void apply_selection(match_list &buf, graph_ty &graph, match_tup &selected) {
    int field = 0;
    for (const auto &elm : selected) {
        // extract the ids
        int j = std::get<0>(elm)->id;
        int t1 = std::get<1>(elm)->id;
        int t2 = std::get<2>(elm)->id;
        // add the match to the results list
        buf.push_back(std::make_tuple<>(field++, j, t1, t2));

        // update array/graph
        graph[t1][t2] = true;
        graph[t2][t1] = true;

        // take action
        std::get<0>(elm)->push(Team::JUDGE);
        std::get<1>(elm)->push(Team::PLAY);
        std::get<2>(elm)->push(Team::PLAY);
    }
}

// Find a team to judge the match
// teams is an ordered list (order: highest priority to judge first)
Team *find_judge(const match_tup &selected, const std::tuple<Team *, Team *> &match,
                 std::vector<Team *> &teams, int nb_match) {
    // check the maximum number of time a team can judge
    int max = nb_match / teams.size() + 1;

    for (auto elm : teams)
        if (elm->count<Team::JUDGE>() < max && std::get<0>(match) != elm && std::get<1>(match) != elm)
            if (compatible(selected, elm))
                return elm;

    return nullptr;
}

// Check if a given team can be used to create a match
// (team not already playing or judging)
bool compatible(const match_tup &selected, const Team *team) {
    // Use a standard algorithm + functor
    // Simply: Goes through the list and returns if the team is never playing nor judging
    return !std::any_of(selected.cbegin(), selected.cend(),
                        [&team](const std::tuple<Team *, Team *, Team *> elm) {
                            return std::get<0>(elm) == team || std::get<1>(elm) == team || std::get<2>(elm) == team;
                        });
}

// Dump the results in human readable way
std::ostream &operator<<(std::ostream &out, const match_list &list) {
    out << "matches:" << std::endl;
    int last = 1;
    int round = 0;
    for (auto it = list.cbegin(); it != list.cend(); it++) {
        if (std::get<0>(*it) <= last) {
            out << "ROUND N°" << round << std::endl;
            round++;
        }
        last = std::get<0>(*it);
        out << "terrain: " << last;
        out << ",\"arbitre\":" << std::get<1>(*it);
        out << ",\"equipes\":";
        out << "[" << std::get<2>(*it) << "," << std::get<3>(*it) << "]" << std::endl;
    }
    return out;
}

// Dump the results in JSON format
void dump(std::ostream &out, const match_list &list) {
    out << "{\"matches\":[";
    for (auto it = list.cbegin(); it != list.cend(); it++) {
        if (it != list.cbegin())
            out << ',';
        out << "{\"terrain\":" << std::get<0>(*it);
        out << ",\"arbitre\":" << std::get<1>(*it);
        out << ",\"equipes\":";
        out << "[" << std::get<2>(*it) << "," << std::get<3>(*it) << "]}";
    }

    out << "]}";
}
