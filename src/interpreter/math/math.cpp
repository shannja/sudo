#include "math.h"
#include "../core/core.h"
#include <cmath>

const QMap<MathOperations, Math::MathOP> Math::operations = {
    {CMD_ADDITION,        Math::add},
    {CMD_SUBTRACTION,     Math::sub},
    {CMD_MULTIPLICATION,  Math::mul},
    {CMD_DIVISION,        Math::div},
    {CMD_RAISE_TO_POWER,  Math::power},
    {CMD_SQUARE_ROOT,     Math::root}
};

/**
 * @brief Executes mathematical operations and coordinates with the Core for storage.
 * * This function follows the "U-Turn" logic:
 * 1. Resolves data from Memory.
 * 2. Processes the data using internal math functions.
 * 3. Creates a NEW instruction to give to the Core Ring to save the result.
 */
QString Math::execute(const Instruction &inst, QMap<QString, Variable> &memory) {
    QList<double> values;

    // 1. ARGUMENT RESOLUTION: Convert Identifiers or Strings into raw numbers (doubles).
    if (!memory.contains(inst.name)) return "[ERROR] Target variable '" + inst.name + "' does not exist.";
    if (memory[inst.name].type != "NUM") {
        return "[ERROR] Target variable '" + inst.name + "' must be a NUM for math operations.";
    }

    for (const Token &t : inst.arguments) {
        // Use the Core's resolution logic to handle variable-vs-literal
        QString resolved = Core::resolveValue(t.value, memory);

        // Catch the specific error signal we designed
        if (resolved.startsWith("[UNDEFINED_REF]:")) {
            return "[ERROR] Math: Undefined variable reference '" + resolved.mid(16) + "'.";
        }

        bool ok;
        double val = resolved.toDouble(&ok);
        if (!ok) return "[ERROR] Math: '" + t.value + "' is not a valid number.";

        values.append(val);
    }

    // 2. OPERATION LOOKUP: Convert the string-based operation into its Enum counterpart.
    MathOperations opType = mathOperationsMap.value(inst.value, CMD_MATH_INVALID);

    // 3. DISPATCH MAP: Routes the operation to the specific helper function.
    if (!operations.contains(opType))
        return "[ERROR] Unknown math operation: " + inst.value;


    // 4. PRE-FLIGHT SAFETY: Catch Division by Zero before entering the math logic.
    if (opType == CMD_DIVISION && values.size() > 1) {
        for (int i = 1; i < values.size(); ++i) {
            if (values[i] == 0) return "[ERROR] Math: Division by zero.";
        }
    }

    if (opType == CMD_SQUARE_ROOT && !values.isEmpty() && values[0] < 0) {
        return "[ERROR] Math: Square root of negative number.";
    }

    if (opType == CMD_RAISE_TO_POWER && values.size() != 2) {
        return "[ERROR] Math: POW requires exactly 2 arguments (base and exponent).";
    }

    if (opType == CMD_SQUARE_ROOT && values.size() != 1) {
        return "[ERROR] Math: ROOT requires exactly 1 argument.";
    }

    // 5. CALCULATION: Execute the math.
    double result = operations[opType](values);

    // 6. WRITE-BACK PROTOCOL:
    // We don't touch memory. We create a "Write-back" instruction for the Core.
    Instruction wb;
    wb.name = inst.name;
    wb.value = QString::number(result, 'g', 15);

    // Since we check for existence at the top, this is always an update.
    wb.action = CMD_UPDATE;
    wb.dataType = CMD_NUMBER;
    wb.category = CMD_VARIABLE;
    wb.isValid = true;

    // Delegate the actual memory write to Core.
    return Core::execute(wb, memory);
}

// --- ARITHMETIC HANDLERS ---

/**
 * Variadic Addition: Sums all provided arguments.
 */
double Math::add(const QList<double>& args) {
    double res = 0;
    for (double v : args) res += v;
    return res;
}

/**
 * Sequential Subtraction: Subtracts subsequent numbers from the first.
 */
double Math::sub(const QList<double>& args) {
    if (args.isEmpty()) return 0;
    double res = args[0];
    for (int i = 1; i < args.size(); ++i) res -= args[i];
    return res;
}

/**
 * Variadic Multiplication.
 */
double Math::mul(const QList<double>& args) {
    if (args.isEmpty()) return 0;
    double res = 1;
    for (double v : args) res *= v;
    return res;
}

/**
 * Sequential Division.
 */
double Math::div(const QList<double>& args) {
    if (args.isEmpty()) return 0;
    double res = args[0];
    for (int i = 1; i < args.size(); ++i) {
        if (args[i] == 0) return 0;
        res /= args[i];
    }
    return res;
}

/**
 * Power: args[0] raised to the power of args[1].
 */
double Math::power(const QList<double>& args) {
    if (args.size() < 2) return 0;
    return std::pow(args[0], args[1]);
}

/**
 * Square Root: Returns sqrt of the first argument.
 */
double Math::root(const QList<double>& args) {
    if (args.isEmpty()) return 0;
    return std::sqrt(args[0]);
}
