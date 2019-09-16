/***********************************************************************************************************************
 *                                                                                                                     *
 * Copyright (C) 2019 Julien Eychenne <jeychenne@gmail.com>                                                            *
 *                                                                                                                     *
 * This software is governed by the CeCILL license under French law and abiding by the rules of distribution of free   *
 * software. You can use, modify and/or redistribute the software under the terms of the CeCILL license as circulated  *
 * by CEA, CNRS and INRIA at the following URL "http://www.cecill.info".                                               *
 *                                                                                                                     *
 * As a counterpart to the access to the source code and rights to copy, modify and redistribute granted by the        *
 * license, users are provided only with a limited warranty and the software's author, the holder of the economic      *
 * rights, and the successive licensors have only limited liability.                                                   *
 *                                                                                                                     *
 * In this respect, the user's attention is drawn to the risks associated with loading, using, modifying and/or        *
 * developing or reproducing the software by the user in light of its specific status of free software, that may mean  *
 * that it is complicated to manipulate, and that also therefore means that it is reserved for developers and          *
 * experienced professionals having in-depth computer knowledge. Users are therefore encouraged to load and test the   *
 * software's suitability as regards their requirements in conditions enabling the security of their systems and/or    *
 * data to be ensured and, more generally, to use and operate it in the same conditions as regards security.           *
 *                                                                                                                     *
 * The fact that you are presently reading this means that you have had knowledge of the CeCILL license and that you   *
 * accept its terms.                                                                                                   *
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
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);
    rule.pattern = QRegularExpression("\'.*\'");
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
