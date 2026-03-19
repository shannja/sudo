#include "logic.h"
#include "../core/core.h"

const QMap<CompareOperations, Logic::CompareOP> Logic::comp_operations = {
    {CMD_EQUAL, Logic::equal},
    {CMD_NEQUAL, Logic::notEqual},
    {CMD_GREATER_THAN, Logic::greaterThan},
    {CMD_LESS_THAN, Logic::lessThan},
    {CMD_GREATER_THAN_EQUAL, Logic::greaterThanEqual},
    {CMD_LESS_THAN_EQUAL, Logic::lessThanEqual}
};

const QMap<LogicOperations, Logic::LogicOP> Logic::logic_operations = {
    {CMD_AND, Logic::andOperator},
    {CMD_NOT, Logic::notOperator},
    {CMD_OR, Logic::orOperator}
};

/**
 * @brief Executes comparision and logical operations and coordinates with the Core for storage.
 * * This function follows the "U-Turn" logic:
 * 1. Resolves data from Memory.
 * 2. Processes the data using internal comparison or logical functions.
 * 3. Creates a NEW instruction to give to the Core to save the result.
 */
QString Logic::execute(const Instruction &inst, QMap<QString, Variable> &memory) {

    // 1. Validate target variable exists and is a BOOL
    if (!memory.contains(inst.name)) return "[ERROR] Logic: Target '" + inst.name + "' not found.";
    if (memory[inst.name].type != "BOOL") return "[ERROR] Logic: Target must be a BOOL variable.";

    // 2. Resolve Arguments using Core::resolveValue
    QString raw1 = Core::resolveValue(inst.arguments[0].value, memory);
    QString raw2 = (inst.arguments.size() > 1) ? Core::resolveValue(inst.arguments[1].value, memory) : "";

    // Check for undefined references
    if (raw1.startsWith("[UNDEFINED_REF]:")) return "[ERROR] Logic: Undefined ref " + raw1.mid(16);
    if (raw2.startsWith("[UNDEFINED_REF]:")) return "[ERROR] Logic: Undefined ref " + raw2.mid(16);

    bool result = false;
    CompareOperations comp_type = compareOperationsMap.value(inst.value, CMD_COMPARE_INVALID);
    LogicOperations lgc_type = logicOperationsMap.value(inst.value, CMD_LOGIC_INVALID);

    // 3. COMPARISON (NUM or STR)
    if (comp_type != CMD_COMPARE_INVALID) {
        if (inst.arguments.size() < 2) return "[ERROR] Comparison requires 2 arguments.";

        bool ok1, ok2;
        double n1 = raw1.toDouble(&ok1);
        double n2 = raw2.toDouble(&ok2);

        if (ok1 && ok2) {
            // Numeric comparison
            result = comp_operations[comp_type](n1, n2);
        } else {
            // String/Literal comparison (Strict: Only EQ and NEQ allowed for non-numbers)
            if (comp_type == CMD_EQUAL) result = (raw1 == raw2);
            else if (comp_type == CMD_NEQUAL) result = (raw1 != raw2);
            else return "[ERROR] Cannot use GT/LT on non-numeric values.";
        }
    }
    // 4. LOGIC (BOOL ONLY)
    else if (lgc_type != CMD_LOGIC_INVALID) {
        bool b1 = (raw1 == "true");
        bool b2 = (raw2 == "true");

        // Strict Check: Ensure we aren't performing logic on numbers/garbage
        if ((raw1 != "true" && raw1 != "false") || (lgc_type != CMD_NOT && raw2 != "true" && raw2 != "false")) {
            return "[ERROR] Logic operators require BOOL values (true/false).";
        }

        result = logic_operations[lgc_type](b1, b2);
    }

    // 5. Write-back (The U-Turn)
    Instruction wb;
    wb.action = CMD_UPDATE;
    wb.name = inst.name;
    wb.value = result ? "true" : "false";
    return Core::execute(wb, memory);
}

// --- COMPARISON HANDLERS ---
/**
 * They only compare str and num then deliver bool values.
 * Comparing boolean values are done by logical handlers.
 */
bool Logic::equal(const double& arg1, const double& arg2){
    return arg1 == arg2;
}

bool Logic::notEqual(const double& arg1, const double& arg2){
    return arg1 != arg2;
}

bool Logic::greaterThan(const double& arg1, const double& arg2){
    return arg1 > arg2;
}

bool Logic::lessThan(const double& arg1, const double& arg2){
    return arg1 < arg2;
}

bool Logic::greaterThanEqual(const double& arg1, const double& arg2){
    return arg1 >= arg2;
}

bool Logic::lessThanEqual(const double& arg1, const double& arg2){
    return arg1 <= arg2;
}

// --- LOGICAL HANDLERS ---
/**
 * They must only compare boolean values.
 */
bool Logic::andOperator(const bool& arg1, const bool& arg2){
    return arg1 && arg2;
}

bool Logic::notOperator(const bool& arg1, const bool& arg2){
    Q_UNUSED(arg2);
    return !arg1;
}

bool Logic::orOperator(const bool& arg1, const bool& arg2){
    return arg1 || arg2;
}



