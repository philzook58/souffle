/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file AstArgument.h
 *
 * Define the classes Argument, Variable, and Constant to represent
 * variables and constants in literals. Variable and Constant are
 * sub-classes of class argument.
 *
 ***********************************************************************/

#pragma once

#include "AstNode.h"
#include "AstTypes.h"
#include "BinaryFunctorOps.h"
#include "FunctorOps.h"
#include "SymbolTable.h"
#include "TernaryFunctorOps.h"
#include "UnaryFunctorOps.h"
#include "Util.h"
#include <array>
#include <cassert>
#include <cstddef>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace souffle {

/* Forward declarations */
class AstLiteral;

/**
 * Intermediate representation of an argument
 */
class AstArgument : public AstNode {
public:
    ~AstArgument() override = default;

    /** Obtains a list of all embedded child nodes */
    std::vector<const AstNode*> getChildNodes() const override {
        return std::vector<const AstNode*>();  // type is just cached, not essential
    }

    /** Create clone */
    AstArgument* clone() const override = 0;
};

/**
 * Subclass of Argument that represents a named variable
 */
class AstVariable : public AstArgument {
protected:
    /** Variable name */
    std::string name;

public:
    AstVariable(std::string n) : AstArgument(), name(std::move(n)) {}

    /** Updates this variable name */
    void setName(const std::string& name) {
        this->name = name;
    }

    /** @return Variable name */
    const std::string& getName() const {
        return name;
    }

    /** Print argument to the given output stream */
    void print(std::ostream& os) const override {
        os << name;
    }

    /** Creates a clone of this AST sub-structure */
    AstVariable* clone() const override {
        AstVariable* res = new AstVariable(name);
        res->setSrcLoc(getSrcLoc());
        return res;
    }

    /** Mutates this node */
    void apply(const AstNodeMapper& /*mapper*/) override {
        // no sub-nodes to consider
    }

protected:
    /** Implements the node comparison for this node type */
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstVariable*>(&node));
        const auto& other = static_cast<const AstVariable&>(node);
        return name == other.name;
    }
};

/**
 * Subclass of Argument that represents an unnamed variable
 */
class AstUnnamedVariable : public AstArgument {
protected:
public:
    AstUnnamedVariable() : AstArgument() {}

    /** Print argument to the given output stream */
    void print(std::ostream& os) const override {
        os << "_";
    }

    /** Creates a clone of this AST sub-structure */
    AstUnnamedVariable* clone() const override {
        auto* res = new AstUnnamedVariable();
        res->setSrcLoc(getSrcLoc());
        return res;
    }

    /** Mutates this node */
    void apply(const AstNodeMapper& /*mapper*/) override {
        // no sub-nodes to consider
    }

protected:
    /** Implements the node comparison for this node type */
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstUnnamedVariable*>(&node));
        return true;
    }
};

/**
 * Subclass of Argument that represents a counter (for projections only)
 */
class AstCounter : public AstArgument {
protected:
public:
    AstCounter() : AstArgument() {}

    /** Print argument to the given output stream */
    void print(std::ostream& os) const override {
        os << "$";
    }

    /** Creates a clone of this AST sub-structure */
    AstCounter* clone() const override {
        auto* res = new AstCounter();
        res->setSrcLoc(getSrcLoc());
        return res;
    }

    /** Mutates this node */
    void apply(const AstNodeMapper& /*mapper*/) override {
        // no sub-nodes to consider within constants
    }

protected:
    /** Implements the node comparison for this node type */
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstCounter*>(&node));
        return true;
    }
};

/**
 * Subclass of Argument that represents a datalog constant value
 */
class AstConstant : public AstArgument {
protected:
    /** Index of this Constant in the SymbolTable */
    AstDomain idx;

public:
    AstConstant(AstDomain i) : AstArgument(), idx(i) {}

    /** @return Return the index of this constant in the SymbolTable */
    AstDomain getIndex() const {
        return idx;
    }

    /** Mutates this node */
    void apply(const AstNodeMapper& /*mapper*/) override {
        // no sub-nodes to consider within constants
    }

protected:
    /** Implements the node comparison for this node type */
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstConstant*>(&node));
        const auto& other = static_cast<const AstConstant&>(node);
        return idx == other.idx;
    }
};

/**
 * Subclass of Argument that represents a datalog constant value
 */
class AstStringConstant : public AstConstant {
    SymbolTable& symTable;
    AstStringConstant(SymbolTable& symTable, size_t index) : AstConstant(index), symTable(symTable) {}

public:
    AstStringConstant(SymbolTable& symTable, const std::string& c)
            : AstConstant(symTable.lookup(c)), symTable(symTable) {}

    /** @return String representation of this Constant */
    const std::string& getConstant() const {
        return symTable.resolve(getIndex());
    }

    /**  Print argument to the given output stream */
    void print(std::ostream& os) const override {
        os << "\"" << getConstant() << "\"";
    }

    /** Creates a clone of this AST sub-structure */
    AstStringConstant* clone() const override {
        auto* res = new AstStringConstant(symTable, getIndex());
        res->setSrcLoc(getSrcLoc());
        return res;
    }
};

/**
 * Subclass of Argument that represents a datalog constant value
 */
class AstNumberConstant : public AstConstant {
public:
    AstNumberConstant(AstDomain num) : AstConstant(num) {}

    /**  Print argument to the given output stream */
    void print(std::ostream& os) const override {
        os << idx;
    }

    /** Creates a clone of this AST sub-structure */
    AstNumberConstant* clone() const override {
        auto* res = new AstNumberConstant(getIndex());
        res->setSrcLoc(getSrcLoc());
        return res;
    }
};

/**
 * Subclass of AstConstant that represents a null-constant (no record)
 */
class AstNullConstant : public AstConstant {
public:
    AstNullConstant() : AstConstant(0) {}

    /**  Print argument to the given output stream */
    void print(std::ostream& os) const override {
        os << '-';
    }

    /** Creates a clone of this AST sub-structure */
    AstNullConstant* clone() const override {
        auto* res = new AstNullConstant();
        res->setSrcLoc(getSrcLoc());
        return res;
    }
};

/**
 * A common base class for AST functors
 */
class AstFunctor : public AstArgument {};


// TODO: fix up the comments on all these bc they seem messed in general
class AstBuiltInFunctor : public AstFunctor {
protected:
    FunctorOp op;
    std::vector<std::unique_ptr<AstArgument>> args;

public:
    // TODO: remove this once subclasses are gone
    AstBuiltInFunctor() = default;

    template <typename... Operands>
    AstBuiltInFunctor(FunctorOp op, Operands... operands) : op(op) {
        std::unique_ptr<AstArgument> tmp[] = {std::move(operands)...};
        for (auto& cur : tmp) {
            args.push_back(std::move(cur));
        }

        assert(getFunctorOpArity(op) == args.size() && "invalid number of arguments for functor");
    }

    // TODO: GET OPERANDS GO HERE

    // TODO: GET FUNCTION GOES HERE

    /** Check if the return value of this functor is a number type. */
    bool isNumerical() const {
        return isNumericFunctorOp(op);
    }

    /** Check if the return value of this functor is a symbol type. */
    bool isSymbolic() const {
        return isSymbolicFunctorOp(op);
    }

    /** Check if the argument of this functor is a number type. */
    bool acceptsNumbers(size_t arg) const {
        return functorOpAcceptsNumbers(arg, op);
    }

    /** Check if the argument of this functor is a symbol type. */
    bool acceptsSymbols(size_t arg) const {
        return functorOpAcceptsSymbols(arg, op);
    }

    /** Print argument to the given output stream */
    void print(std::ostream& os) const override {
        os << getSymbolForFunctorOp(op);
        os << "(";
        os << join(args, ",", print_deref<std::unique_ptr<AstArgument>>());
        os << ")";
    }

    // TODO: clone

    /** Mutates this node */
    void apply(const AstNodeMapper& map) override {
        for (size_t i = 0; i < args.size(); i++) {
            args[i] = map(std::move(args[i]));
        }
    }

    // TODO: get child nodes

protected:
    // TODO: EQUAL
};

/**
 * Subclass of Argument that represents a unary function
 */
class AstUnaryFunctor : public AstBuiltInFunctor {
public:
    AstUnaryFunctor(UnaryOp fun, std::unique_ptr<AstArgument> o) : AstBuiltInFunctor(getFunctorOpForSymbol(getSymbolForUnaryOp(fun)), std::move(o)) {}
    AstUnaryFunctor(FunctorOp fun, std::unique_ptr<AstArgument> o) : AstBuiltInFunctor(fun, std::move(o)) {}

    ~AstUnaryFunctor() override = default;

    AstArgument* getOperand() const {
        return args[0].get();
    }

    UnaryOp getFunction() const {
        return getUnaryOpForSymbol(getSymbolForFunctorOp(op));
    }

    /** Creates a clone */
    AstUnaryFunctor* clone() const override {
        auto res = new AstUnaryFunctor(op, std::unique_ptr<AstArgument>(args[0]->clone()));
        res->setSrcLoc(getSrcLoc());
        return res;
    }

    /** Obtains a list of all embedded child nodes */
    std::vector<const AstNode*> getChildNodes() const override {
        auto res = AstArgument::getChildNodes();
        res.push_back(args[0].get());
        return res;
    }

protected:
    /** Implements the node comparison for this node type */
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstUnaryFunctor*>(&node));
        const auto& other = static_cast<const AstUnaryFunctor&>(node);
        return op == other.op && *args[0] == *other.args[0];
    }
};

/**
 * Subclass of Argument that represents a binary function
 */
class AstBinaryFunctor : public AstBuiltInFunctor {
public:
    AstBinaryFunctor(BinaryOp fun, std::unique_ptr<AstArgument> l, std::unique_ptr<AstArgument> r)
            : AstBuiltInFunctor(getFunctorOpForSymbol(getSymbolForBinaryOp(fun)), std::move(l), std::move(r)) {}
    AstBinaryFunctor(FunctorOp fun, std::unique_ptr<AstArgument> l, std::unique_ptr<AstArgument> r)
            : AstBuiltInFunctor(fun, std::move(l), std::move(r)) {}

    ~AstBinaryFunctor() override = default;

    AstArgument* getLHS() const {
        return args[0].get();
    }

    AstArgument* getRHS() const {
        return args[1].get();
    }

    BinaryOp getFunction() const {
        return getBinaryOpForSymbol(getSymbolForFunctorOp(op));
    }

    /** Print argument to the given output stream */
    void print(std::ostream& os) const override {
        // TODO: FIX THIS for new version - just really need to add an IF statmenet for CAT? BUT ALSO CHECK IF MIN WAS DONE
        // TODO: maybe add infix method
        if (op < FunctorOp::MAX) {
            os << "(";
            args[0]->print(os);
            os << getSymbolForFunctorOp(op);
            args[1]->print(os);
            os << ")";
        } else {
            os << getSymbolForFunctorOp(op);
            os << "(";
            args[0]->print(os);
            os << ",";
            args[1]->print(os);
            os << ")";
        }
    }

    /** Creates a clone */
    AstBinaryFunctor* clone() const override {
        auto res = new AstBinaryFunctor(op, std::unique_ptr<AstArgument>(args[0]->clone()),
                std::unique_ptr<AstArgument>(args[1]->clone()));
        res->setSrcLoc(getSrcLoc());
        return res;
    }

    /** Obtains a list of all embedded child nodes */
    std::vector<const AstNode*> getChildNodes() const override {
        auto res = AstArgument::getChildNodes();
        res.push_back(args[0].get());
        res.push_back(args[1].get());
        return res;
    }

protected:
    /** Implements the node comparison for this node type */
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstBinaryFunctor*>(&node));
        const auto& other = static_cast<const AstBinaryFunctor&>(node);
        return op == other.op && *args[0] == *other.args[0] && *args[1] == *other.args[1];
    }
};

/**
 * @class TernaryFunctor
 * @brief Subclass of Argument that represents a binary functor
 */
// TODO: REPRESETNS A BINARY FUNCTOR?
class AstTernaryFunctor : public AstBuiltInFunctor {
public:
    AstTernaryFunctor(FunctorOp fun, std::unique_ptr<AstArgument> a1, std::unique_ptr<AstArgument> a2,
            std::unique_ptr<AstArgument> a3)
            : AstBuiltInFunctor(fun, std::move(a1), std::move(a2), std::move(a3)) {}
    AstTernaryFunctor(TernaryOp fun, std::unique_ptr<AstArgument> a1, std::unique_ptr<AstArgument> a2,
            std::unique_ptr<AstArgument> a3)
            : AstBuiltInFunctor(getFunctorOpForSymbol(getSymbolForTernaryOp(fun)), std::move(a1), std::move(a2),
                      std::move(a3)) {}

    ~AstTernaryFunctor() override = default;

    AstArgument* getArg(int idx) const {
        assert(idx >= 0 && idx < 3 && "wrong argument");
        return args[idx].get();
    }

    TernaryOp getFunction() const {
        return getTernaryOpForSymbol(getSymbolForFunctorOp(op));
    }

    /** Clone this node  */
    AstTernaryFunctor* clone() const override {
        auto res = new AstTernaryFunctor(op, std::unique_ptr<AstArgument>(args[0]->clone()),
                std::unique_ptr<AstArgument>(args[1]->clone()),
                std::unique_ptr<AstArgument>(args[2]->clone()));
        res->setSrcLoc(getSrcLoc());
        return res;
    }

    /** Obtains a list of all embedded child nodes */
    std::vector<const AstNode*> getChildNodes() const override {
        auto res = AstArgument::getChildNodes();
        res.push_back(args[0].get());
        res.push_back(args[1].get());
        res.push_back(args[2].get());
        return res;
    }

protected:
    /** Implements the node comparison for this node type */
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstTernaryFunctor*>(&node));
        const auto& other = static_cast<const AstTernaryFunctor&>(node);
        return op == other.op && *args[0] == *other.args[0] && *args[1] == *other.args[1] &&
               *args[2] == *other.args[2];
    }
};

/**
 * Subclass of Argument that represents a unary function
 */
// TODO: SUBCLASS OF UNARY FUCNTION?
class AstUserDefinedFunctor : public AstFunctor {
protected:
    /** name of user-defined functor */
    std::string name;

    /** arguments of user-defined functor */
    std::vector<std::unique_ptr<AstArgument>> args;

public:
    AstUserDefinedFunctor() {}

    AstUserDefinedFunctor(const std::string& name) : name(name) {}

    ~AstUserDefinedFunctor() override = default;

    /** get name */
    const std::string& getName() const {
        return name;
    }

    /** set name */
    void setName(const std::string& n) {
        name = n;
    }

    /** get argument */
    const AstArgument* getArg(size_t idx) const {
        return args[idx].get();
    }

    /** get number of arguments */
    size_t getArgCount() const {
        return args.size();
    }

    /** get arguments */
    std::vector<AstArgument*> getArguments() const {
        return toPtrVector(args);
    }

    /** add argument to argument list */
    void add(std::unique_ptr<AstArgument> arg) {
        args.push_back(std::move(arg));
    }

    /** print user-defined functor */
    void print(std::ostream& os) const override {
        os << '@' << name << "(" << join(args, ",", print_deref<std::unique_ptr<AstArgument>>()) << ")";
    }

    /** Create clone */
    AstUserDefinedFunctor* clone() const override {
        auto res = new AstUserDefinedFunctor();
        for (auto& cur : args) {
            res->args.push_back(std::unique_ptr<AstArgument>(cur->clone()));
        }
        res->setSrcLoc(getSrcLoc());
        res->setName(getName());
        return res;
    }

    /** Mutates this node */
    void apply(const AstNodeMapper& map) override {
        for (auto& arg : args) {
            arg = map(std::move(arg));
        }
    }

    /** Obtains a list of all embedded child nodes */
    std::vector<const AstNode*> getChildNodes() const override {
        auto res = AstArgument::getChildNodes();
        for (auto& cur : args) {
            res.push_back(cur.get());
        }
        return res;
    }

protected:
    /** Implements the node comparison for this node type */
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstUserDefinedFunctor*>(&node));
        const auto& other = static_cast<const AstUserDefinedFunctor&>(node);
        return name == other.name && equal_targets(args, other.args);
    }
};

/**
 * An argument that takes a list of values and combines them into a
 * new record.
 */
class AstRecordInit : public AstArgument {
    /** The list of components to be aggregated into a record */
    std::vector<std::unique_ptr<AstArgument>> args;

public:
    AstRecordInit() = default;

    ~AstRecordInit() override = default;

    void add(std::unique_ptr<AstArgument> arg) {
        args.push_back(std::move(arg));
    }

    std::vector<AstArgument*> getArguments() const {
        return toPtrVector(args);
    }

    void print(std::ostream& os) const override {
        os << "[" << join(args, ",", print_deref<std::unique_ptr<AstArgument>>()) << "]";
    }

    /** Creates a clone of this AST sub-structure */
    AstRecordInit* clone() const override {
        auto res = new AstRecordInit();
        for (auto& cur : args) {
            res->args.push_back(std::unique_ptr<AstArgument>(cur->clone()));
        }
        res->setSrcLoc(getSrcLoc());
        return res;
    }

    /** Mutates this node */
    void apply(const AstNodeMapper& map) override {
        for (auto& arg : args) {
            arg = map(std::move(arg));
        }
    }

    /** Obtains a list of all embedded child nodes */
    std::vector<const AstNode*> getChildNodes() const override {
        auto res = AstArgument::getChildNodes();
        for (auto& cur : args) {
            res.push_back(cur.get());
        }
        return res;
    }

protected:
    /** Implements the node comparison for this node type */
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstRecordInit*>(&node));
        const auto& other = static_cast<const AstRecordInit&>(node);
        return equal_targets(args, other.args);
    }
};

/**
 * An argument capable of casting a value of one type into another.
 */
class AstTypeCast : public AstArgument {
    /** The value to be casted */
    std::unique_ptr<AstArgument> value;

    /** The target type name */
    std::string type;

public:
    AstTypeCast(std::unique_ptr<AstArgument> value, std::string type)
            : value(std::move(value)), type(std::move(type)) {}

    void print(std::ostream& os) const override {
        os << *value << " as " << type;
    }

    AstArgument* getValue() const {
        return value.get();
    }

    const std::string& getType() const {
        return type;
    }

    /** Obtains a list of all embedded child nodes */
    std::vector<const AstNode*> getChildNodes() const override {
        auto res = AstArgument::getChildNodes();
        res.push_back(value.get());
        return res;
    }

    /** Creates a clone of this AST sub-structure */
    AstTypeCast* clone() const override {
        auto res = new AstTypeCast(std::unique_ptr<AstArgument>(value->clone()), type);
        res->setSrcLoc(getSrcLoc());
        return res;
    }

    /** Mutates this node */
    void apply(const AstNodeMapper& map) override {
        value = map(std::move(value));
    }

protected:
    /** Implements the node comparison for this node type */
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstTypeCast*>(&node));
        const auto& other = static_cast<const AstTypeCast&>(node);
        return type == other.type && *value == *other.value;
    }
};

/**
 * An argument aggregating a value from a sub-query.
 */
class AstAggregator : public AstArgument {
public:
    /**
     * The kind of utilized aggregation operator.
     * Note: lower-case is utilized due to a collision with
     *  constants in the parser.
     */
    enum Op { min, max, count, sum };

private:
    /** The aggregation operator of this aggregation step */
    Op fun;

    /** The expression to be aggregated */
    std::unique_ptr<AstArgument> expr;

    /** A list of body-literals forming a sub-query which's result is projected and aggregated */
    std::vector<std::unique_ptr<AstLiteral>> body;

public:
    /** Creates a new aggregation node */
    AstAggregator(Op fun) : fun(fun), expr(nullptr) {}

    /** Destructor */
    ~AstAggregator() override = default;

    // -- getters and setters --

    Op getOperator() const {
        return fun;
    }

    void setTargetExpression(std::unique_ptr<AstArgument> arg) {
        expr = std::move(arg);
    }

    const AstArgument* getTargetExpression() const {
        return expr.get();
    }

    std::vector<AstLiteral*> getBodyLiterals() const {
        return toPtrVector(body);
    }

    void clearBodyLiterals() {
        body.clear();
    }

    void addBodyLiteral(std::unique_ptr<AstLiteral> lit) {
        body.push_back(std::move(lit));
    }

    // -- others --

    /** Prints this instance in a parse-able format */
    void print(std::ostream& os) const override;

    /** Obtains a list of all embedded child nodes */
    std::vector<const AstNode*> getChildNodes() const override;

    /** Creates a clone of this AST sub-structure */
    AstAggregator* clone() const override;

    /** Mutates this node */
    void apply(const AstNodeMapper& map) override {
        if (expr) {
            expr = map(std::move(expr));
        }
        for (auto& cur : body) {
            cur = map(std::move(cur));
        }
    }

protected:
    /** Implements the node comparison for this node type */
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstAggregator*>(&node));
        const auto& other = static_cast<const AstAggregator&>(node);
        return fun == other.fun && equal_ptr(expr, other.expr) && equal_targets(body, other.body);
    }
};

/**
 * An argument taking its value from an argument of a RAM subroutine
 */
class AstSubroutineArgument : public AstArgument {
private:
    /** Index of argument in argument list*/
    size_t number;

public:
    AstSubroutineArgument(size_t n) : AstArgument(), number(n) {}

    /** Return argument number */
    size_t getNumber() const {
        return number;
    }

    /** Print argument to the given output stream */
    void print(std::ostream& os) const override {
        os << "arg_" << number;
    }

    /** Creates a clone of this AST sub-structure */
    AstSubroutineArgument* clone() const override {
        auto* res = new AstSubroutineArgument(number);
        res->setSrcLoc(getSrcLoc());
        return res;
    }

    /** Mutates this node */
    void apply(const AstNodeMapper& /*mapper*/) override {
        // no sub-nodes to consider
    }

protected:
    /** Implements the node comparison for this node type */
    bool equal(const AstNode& node) const override {
        assert(nullptr != dynamic_cast<const AstSubroutineArgument*>(&node));
        const auto& other = static_cast<const AstSubroutineArgument&>(node);
        return number == other.number;
    }
};

}  // end of namespace souffle
