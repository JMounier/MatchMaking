#pragma once

#include "structures.hh"

match_list make_match(int, int);
graph_ty gen_matches(int nb_team);
teams_list gen_teams(int nb_team);
void gen_round(match_list& buf, graph_ty& graph, teams_list& teams, int nb_field);
int count_left(const graph_ty& g, int max);
bool done(const graph_ty&);
std::vector<std::tuple<Team*, Team*>> list_remaining(const graph_ty& g, teams_list& teams);
Team* find_judge(const match_tup&, const std::tuple<Team*, Team*>&,
        std::vector<Team*>&, int);
bool compatible(const match_tup&,const Team*);
void apply_sleep(teams_list&, int);
void apply_selection(match_list&, graph_ty&, match_tup&);

std::ostream& operator<<(std::ostream&, const match_list&);
void dump(std::ostream&, const match_list&);

