#ifndef BASE__CONSTANT_CONDITION_H_
#define BASE__CONSTANT_CONDITION_H_

#include "base/graph/condition/condition.h"

class ConstantCondition : public Condition {
public:
    const bool constant;

    ConstantCondition (bool constant) : constant(constant) { }

    bool eval(Binding&) {
        return constant;
    }

    ConditionType type() {
        return ConditionType::constant;
    }

    void check_names(std::map<std::string, ObjectType>& ) { }
};

#endif // BASE__CONSTANT_CONDITION_H_
