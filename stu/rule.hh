#ifndef RULE_HH
#define RULE_HH

/* 
 * Data structures for representing rules. 
 */

#include <unordered_map>

#include "token.hh"

/* 
 * A rule.  The class Rule allows parameters; there is no
 * "unparametrized rule" class. 
 */ 
class Rule
{
public:

	/* The targets of the rule, in the order specified in the rule.  
	 * Contains at least one element. 
	 * Each element contains all parameters of the rule,
	 * and therefore should be used for iterating of all parameters.
	 * The place in each target is used when referring to a target
	 * specifically.  */ 
	const vector <shared_ptr <Place_Param_Target> > place_param_targets; 

	/* The dependencies in order of declaration.  Dependencies are
	 * included multiple times if they appear multiple times in the
	 * source.  Any parameter occuring any dependency also
	 * occurs in every target. */ 
	vector <shared_ptr <Dependency> > dependencies;

	/* The place of the rule as a whole.  Taken from the place of
	 * the first target (but could be different, in principle)  */   
	const Place place;

	/* 
	 * The command (optional).  Contains its own place, as it is a
	 * token.  Null when the rule does not have a command, i.e.,
	 * ends in a semicolon ';'.  For hardcoded rules, the content of
	 * the file (not optional). 
	 */  
	const shared_ptr <const Command> command;

	/* 
	 * When !is_copy:  The name of the file from which
	 *   input should be read; must be one of the file dependencies.
	 *   Empty for no input redirection.   
	 * When is_copy: the file from which to copy; never empty.
	 */ 
	const Name filename; 

	/* Index within PLACE_PARAM_TARGETS of the target to which
	 * output redirection is applied. -1 if no output redirection is
	 * used. The target with that index is a file target. */
	const int redirect_index; 

	/* Whether the command is a command or hardcoded content */ 
	const bool is_hardcode;

	/* Whether the rule is a copy rule, i.e., declared with '='
	 * followed by a filename. */ 
	const bool is_copy;

	/* Direct constructor that specifies everything */
	Rule(vector <shared_ptr <Place_Param_Target> > &&place_param_targets,
	     vector <shared_ptr <Dependency> > &&dependencies_,
	     const Place &place_,
	     const shared_ptr <const Command> &command_,
	     Name &&filename_,
	     bool is_hardcode_,
	     int redirect_index_,
	     bool is_copy_); 

	/* Regular rule:  all cases execpt copy rules */
	Rule(vector <shared_ptr <Place_Param_Target> > &&place_param_targets_,
	     const vector <shared_ptr <Dependency> > &dependencies_,
	     shared_ptr <const Command> command_,
	     bool is_hardcode_,
	     int redirect_index_,
	     const Name &filename_input_);

	/* A copy rule.  When the places are EMPTY, the corresponding
	 * flag is not used. */
	Rule(shared_ptr <Place_Param_Target> place_param_target_,
	     shared_ptr <Place_Name> place_name_source_,
	     const Place &place_persistent,
	     const Place &place_optional); 

	/* Whether the rule is parametrized */ 
	bool is_parametrized() const {
		return place_param_targets.front()->place_name.get_n() != 0; 
	}

	/* Format the rule, as for the -P option */ 
	string format_out() const; 

	const vector <string> &get_parameters() const
	{
		assert(place_param_targets.size() != 0); 
		return place_param_targets.front()->place_name.get_parameters(); 
	}

	/* Return the same rule as RULE, but with parameters having been
	 * replaced by the given MAPPING.  
	 * We pass THIS as PARAM_RULE explicitly so we can return it
	 * itself when it is unparametrized.  */ 
	static shared_ptr <Rule> instantiate(shared_ptr <Rule> rule,
					     const map <string, string> &mapping);
};

/* 
 * A set of parametrized rules. 
 */
class Rule_Set
{
private:

	/* All unparametrized rules by their target.  Rules
	 * with multiple targets are included multiple times, for each
	 * of their targets. */ 
	unordered_map <Target, shared_ptr <Rule> > rules_unparametrized;

	/* All parametrized rules. */ 
	vector <shared_ptr <Rule> > rules_parametrized;

public:

	/* Add rules to this rule set.  
	 * While adding rules, check for duplicates, and print and throw
	 * a logical error if there is. 
	 * If the given rule has duplicate targets, print and throw a
	 * logical error.  */ 
	void add(vector <shared_ptr <Rule> > &rules_);

	/* Match TARGET to a rule, and return the instantiated
	 * (unparametrized) corresponding rule.  TARGET must be
	 * non-dynamic.  MAPPING_OUT must be empty. 
	 * Return null when no match is found. 
	 * When a match is found, write the original rule into
	 * ORIGINAL_RULE and the matched parameters into MAPPING_OUT.   
	 * Throws errors. 
	 * PLACE is the place of the dependency; used in error messages.  */ 
	shared_ptr <Rule> get(Target target, 
			      shared_ptr <Rule> &rule_original,
			      map <string, string> &mapping_out,
			      const Place &place);

	/* Print the rule set to standard output, as used in the -P option */  
	void print() const;
};

Rule::Rule(vector <shared_ptr <Place_Param_Target> > &&place_param_targets_,
	   vector <shared_ptr <Dependency> > &&dependencies_,
	   const Place &place_,
	   const shared_ptr <const Command> &command_,
	   Name &&filename_,
	   bool is_hardcode_,
	   int redirect_index_,
	   bool is_copy_)
	:  place_param_targets(place_param_targets_),
	   dependencies(dependencies_),
	   place(place_),
	   command(command_),
	   filename(filename_),
	   redirect_index(redirect_index_),
	   is_hardcode(is_hardcode_),
	   is_copy(is_copy_)
{  }

Rule::Rule(vector <shared_ptr <Place_Param_Target> > &&place_param_targets_,
	   const vector <shared_ptr <Dependency> > &dependencies_,
	   shared_ptr <const Command> command_,
	   bool is_hardcode_,
	   int redirect_index_,
	   const Name &filename_)
	:  place_param_targets(place_param_targets_), 
	   dependencies(dependencies_),
  	   place(place_param_targets_[0]->place),
	   command(command_),
	   filename(filename_),
	   redirect_index(redirect_index_),
	   is_hardcode(is_hardcode_),
	   is_copy(false)
{ 
	assert(place_param_targets.size() != 0); 
	assert(redirect_index>= -1);
	assert(redirect_index < (ssize_t) place_param_targets.size());
	if (redirect_index >= 0)
		assert(place_param_targets[redirect_index]->type == Type::FILE); 

	/* Check that all dependencies only include
	 * parameters from the target */ 
	set <string> parameters;
	for (auto &parameter:  get_parameters()) {
		parameters.insert(parameter); 
	}

	/* Check that only valid parameters are used */ 
	for (auto &i:  dependencies) {

		shared_ptr <Dependency> dep= i;
		while (dynamic_pointer_cast <Dynamic_Dependency> (dep)) {
			dep= dynamic_pointer_cast <Dynamic_Dependency> (dep)->dependency;
		}

		if (dynamic_pointer_cast <Direct_Dependency> (dep)) {

			shared_ptr <Direct_Dependency> dependency= 
				dynamic_pointer_cast <Direct_Dependency> (dep); 

			for (unsigned jj= 0;  
			     jj < dependency->place_param_target.place_name.get_n();
			     ++jj) {
				string parameter= dependency->place_param_target
					.place_name.get_parameters()[jj]; 
				if (parameters.count(parameter) == 0) {
					dependency->place_param_target
						.place_name.get_places()[jj] <<
						fmt("parameter %s must not appear in dependency %s", 
						    prefix_format_word(parameter, "$"),
						    dependency->place_param_target.format_word());
					if (place_param_targets.size() == 1) {
						place_param_targets[0]->place <<
							fmt("because it does not appear in target %s",
							    place_param_targets[0]->format_word());
					} else {
						place << fmt("because it does not appear in any of the targets %s... of the rule",
							     place_param_targets[0]->format_word()); 
					}
					throw ERROR_LOGICAL; 
				}
			}
		} else {
			assert(false); 
		}
	}
}

Rule::Rule(shared_ptr <Place_Param_Target> place_param_target_,
	   shared_ptr <Place_Name> place_name_source_,
	   const Place &place_persistent,
	   const Place &place_optional)
	:  place_param_targets{place_param_target_},
	   place(place_param_target_->place),
	   filename(*place_name_source_),
	   redirect_index(-1),
	   is_hardcode(false),
	   is_copy(true)
{
	auto dependency= 
		make_shared <Direct_Dependency> 
		(0, Place_Param_Target(Type::FILE, *place_name_source_));

	if (! place_persistent.empty()) {
		dependency->flags |= F_PERSISTENT;
		dependency->places[I_PERSISTENT]= place_persistent;
	}
	if (! place_optional.empty()) {
		dependency->flags |= F_OPTIONAL;
		dependency->places[I_OPTIONAL]= place_optional;
	}

	dependencies.push_back(dependency);
}

shared_ptr <Rule> 
Rule::instantiate(shared_ptr <Rule> rule,
		  const map <string, string> &mapping) 
{
	/* The rule is unparametrized -- return it */ 
	if (rule->get_parameters().size() == 0) {
		return rule;
	}

	vector <shared_ptr <Place_Param_Target> > 
		place_param_targets(rule->place_param_targets.size());
	for (unsigned i= 0;  i < rule->place_param_targets.size();  ++i) 
		place_param_targets[i]= rule->place_param_targets[i]->instantiate(mapping);

	vector <shared_ptr <Dependency> > dependencies;
	for (auto &dependency:  rule->dependencies) {
		dependencies.push_back(dependency->instantiate(mapping));
	}

	return make_shared <Rule> 
		(move(place_param_targets),
		 move(dependencies),
		 rule->place,
		 rule->command,
		 move(rule->filename.instantiate(mapping)),
		 rule->is_hardcode,
		 rule->redirect_index,
		 rule->is_copy); 
}

string Rule::format_out() const
{
	string ret;

	ret += "Rule(";
	
	bool first= true;
	for (auto place_param_target:  place_param_targets) {
		if (first)
			first= false;
		else
			ret += ' '; 
		ret += place_param_target->format_out(); 
	}

	if (dependencies.size() != 0)
		ret += ": ";
	for (auto i= dependencies.begin();  i != dependencies.end();  ++i) {
		if (i != dependencies.begin())
			ret += ", ";
		ret += (*i)->format_out(); 
	}

	ret += ")";

	return ret; 
}

void Rule_Set::add(vector <shared_ptr <Rule> > &rules_) 
{
	for (auto &rule:  rules_) {

		/* Check that the rule doesn't have a duplicate target */ 
		for (unsigned i= 0;  i < rule->place_param_targets.size();  ++i) {
			for (unsigned j= 0;  j < i;  ++j) {
				if (*rule->place_param_targets[i] ==
				    *rule->place_param_targets[j]) {
					rule->place_param_targets[i]->place << 
						fmt("there must not be a target %s",
						    rule->place_param_targets[i]->format_word()); 
					rule->place_param_targets[j]->place << 
						fmt("shadowing target %s of the same rule",
						    rule->place_param_targets[j]->format_word()); 
					throw ERROR_LOGICAL; 
				}
			}
		}

		if (! rule->is_parametrized()) {
			for (auto place_param_target:  rule->place_param_targets) {
				Target target= place_param_target->unparametrized(); 
				if (rules_unparametrized.count(target)) {
					place_param_target->place <<
						fmt("there must not be a second rule for target %s", 
						    target.format_word());
					auto rule_2= rules_unparametrized.at(target); 
					for (auto place_param_target_2: rule_2->place_param_targets) {
						assert(place_param_target_2->place_name.get_n() == 0);
						if (place_param_target_2->unparametrized() == target) {
							place_param_target_2->place << 
								fmt("shadowing previous rule %s", 
								    target.format_word());  
							break;
						}
					}
					throw ERROR_LOGICAL; 
				}
				rules_unparametrized[target]= rule;
			}
		} else {
			rules_parametrized.push_back(rule); 
		}
	}
}

shared_ptr <Rule> Rule_Set::get(Target target, 
				shared_ptr <Rule> &rule_original,
				map <string, string> &mapping_out,
				const Place &place)
{
	assert(target.type == Type::FILE || target.type == Type::TRANSIENT); 
	assert(mapping_out.size() == 0); 

	/* Check for an unparametrized rule.  Since we keep them in a
	 * map by target filename(s), there can only be a single matching rule to
	 * begin with.  (I.e., if multiple unparametrized rules for the same
	 * filename exist, then that error is caught earlier when the
	 * Rule_Set is built.)  */ 
	auto i= rules_unparametrized.find(target);
	if (i != rules_unparametrized.end()) {

		shared_ptr <Rule> rule= i->second;
		assert(rule != nullptr); 
		assert(rule->place_param_targets.front()->place_name.get_n() == 0);
#ifndef NDEBUG		
		/* Check that the target is a target of the found
		 * rule */
		bool found= false;
		for (auto place_param_target:  rule->place_param_targets) {
			if (place_param_target->unparametrized() == target)
				found= true;
		}
		assert(found); 
#endif 

		rule_original= rule; 
		return rule;
	}

	/* Search the best parametrized rule.  Since this implementation
	 * does not have an index for parametrized rules, we simply
	 * check all rules, and choose the best-fitting one.  */ 

	/* Element [0] corresponds to the best rule. */ 
	vector <shared_ptr <Rule> > rules_best;
	vector <map <string, string> > mappings_best; 
	vector <vector <unsigned> > anchorings_best; 
	vector <shared_ptr <Place_Param_Target> > place_param_targets_best; 

	for (auto &rule:  rules_parametrized) {

		for (auto &place_param_target:  rule->place_param_targets) {

			assert(place_param_target->place_name.get_n() > 0);
		
			map <string, string> mapping;
			vector <unsigned> anchoring;

			/* The parametrized rule is of another type */ 
			if (target.type != place_param_target->type)
				continue;

			/* The parametrized rule does not match */ 
			if (! place_param_target->place_name
			    .match(target.name, mapping, anchoring))
				continue; 

			assert(anchoring.size() == 
			       (2 * place_param_target->place_name.get_n())); 

			size_t k= rules_best.size(); 
			assert(k == anchorings_best.size()); 
			assert(k == mappings_best.size()); 

			/* Check whether the rule is dominated by at least one other rule */
			for (int j= 0;  j < (ssize_t) k;  ++j) {
				if (Name::anchoring_dominates
				    (anchorings_best[j], anchoring)) {
					goto dont_add;
				}
			}

			/* Check whether the rule dominates all other rules */ 
			{
				bool is_best= true;
				for (int j= 0;  is_best && j < (ssize_t) k;  ++j) {
					if (! Name::anchoring_dominates(anchoring, anchorings_best[j]))
						is_best= false;
				}
				if (is_best) {
					k= 0;
				}
			} 
			rules_best.resize(k+1); 
			mappings_best.resize(k+1);
			anchorings_best.resize(k+1); 
			place_param_targets_best.resize(k+1); 
			rules_best[k]= rule;
			swap(mapping, mappings_best[k]);
			swap(anchoring, anchorings_best[k]); 
			place_param_targets_best[k]= place_param_target;
		dont_add:;
		}
	}

	/* No rule matches */ 
	if (rules_best.size() == 0) {
		assert(rules_best.size() == 0); 
		return nullptr; 
	}
	assert(rules_best.size() >= 1);

	/* More than one rule matches:  error */ 
	if (rules_best.size() > 1) {
		place << fmt("multiple minimal rules for target %s", 
			     target.format_word());
		for (auto &place_param_target:  place_param_targets_best) {
			place_param_target->place <<
				fmt("rule with target %s", 
				    place_param_target->format_word()); 
		}
		throw ERROR_LOGICAL; 
	}
	assert(rules_best.size() == 1); 

	/* Instantiate the rule */ 
	shared_ptr <Rule> rule_best= rules_best[0];
	swap(mapping_out, mappings_best[0]); 
	rule_original= rule_best; 

	shared_ptr <Rule> ret
		(Rule::instantiate(rule_best, mapping_out));
		
	return ret;
}

void Rule_Set::print() const
{
	for (auto i:  rules_unparametrized)  {
		string text= i.second->format_out(); 
		puts(text.c_str()); 
	}

	for (auto i:  rules_parametrized)  {
		string text= i->format_out(); 
		puts(text.c_str()); 
	}
}

#endif /* ! RULE_HH */
