#pragma once
#include <QWidget>
#include <QScrollArea>
#include <vector>
#include <set>
#include <string>
#include "../core/Parser.h"

class GrammarCoverage : public QWidget {
    Q_OBJECT
public:
    explicit GrammarCoverage(QWidget* parent = nullptr);
    void setDerivationSteps(const std::vector<DerivationStep>& steps);
    void resetCoverage();

protected:
    void paintEvent(QPaintEvent*) override;

private:
    struct Rule {
        QString id;       // "R1", "R2", …
        QString lhs;
        QString rhs;
        bool    used = false;
        int     useCount = 0;
    };

    void buildRules();
    void markUsed(const std::string& productionRule);

    std::vector<Rule> rules;
    std::set<std::string> usedRuleIds;
    int totalUsed = 0;
};
