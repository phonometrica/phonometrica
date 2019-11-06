/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public   *
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any      *
 * later version.                                                                                                      *
 *                                                                                                                     *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied  *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more       *
 * details.                                                                                                            *
 *                                                                                                                     *
 * You should have received a copy of the GNU General Public License along with this program. If not, see              *
 * <http://www.gnu.org/licenses/>.                                                                                     *
 *                                                                                                                     *
 * Created: 28/02/2019                                                                                                 *
 *                                                                                                                     *
 * Purpose: see header.                                                                                                *
 *                                                                                                                     *
 ***********************************************************************************************************************/

#include <phon/gui/highlighter.hpp>

namespace phonometrica {


Highlighter::Highlighter(QTextDocument *parent) :
    QSyntaxHighlighter(parent)
{
    HighlightingRule rule;
    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywords;
    keywords <<  "\\band\\b" << "\\bbreak\\b" << "\\bcontinue\\b" << "\\bdebug\\b" << "\\bdo\\b" << "\\bdownto\\b"
        << "\\belse\\b" << "\\belsif\\b" << "\\bend\\b" << "\\bexport\\b" << "\\bfor\\b" << "\\bforeach\\b"
        << "\\bfunction\\b" << "\\bget\\b" << "\\bif\\b" << "\\bimport\\b" << "\\bin\\b" << "\\binstanceof\\b"
        << "\\bis\\b" << "\\bnew\\b" << "\\bnot\\b" << "\\bor\\b" << "\\bpass\\b" << "\\brepeat\\b" << "\\breturn\\b"
        << "\\bset\\b" << "\\bstep\\b" << "\\bthen\\b" << "\\bto\\b" << "\\btypeof\\b" << "\\buntil\\b" << "\\bvar\\b"
        << "\\bwhile\\b";

    for (const QString &pattern : keywords)
    {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    constantFormat.setFontWeight(QFont::Bold);
    constantFormat.setForeground(Qt::darkYellow);
    rule.pattern = QRegularExpression("\\b(this|true|false|null|undefined)\\b");
    rule.format = constantFormat;
    highlightingRules.append(rule);

    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegularExpression("\\bQ[A-Za-z]+\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);

    quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression("\".*?\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);
    rule.pattern = QRegularExpression("\'.*?\'");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    singleLineCommentFormat.setForeground(Qt::darkGray);
    rule.pattern = QRegularExpression("#[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(Qt::darkGray);

    commentStartExpression = QRegularExpression("!\\*");
    commentEndExpression = QRegularExpression("\\*!");
}

void Highlighter::highlightBlock(const QString &text)
{
    for(const HighlightingRule &rule : highlightingRules)
    {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext())
        {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);

   while (startIndex >= 0)
   {
       QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
       int endIndex = match.capturedStart();
       int commentLength = 0;

       if (endIndex == -1)
       {
           setCurrentBlockState(1);
           commentLength = text.length() - startIndex;
       }
       else
       {
           commentLength = endIndex - startIndex + match.capturedLength();
       }

       setFormat(startIndex, commentLength, multiLineCommentFormat);
       startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
   }
}


} // phonometrica
