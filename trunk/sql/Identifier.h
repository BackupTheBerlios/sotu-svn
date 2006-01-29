/*
  The contents of this file are subject to the Initial Developer's Public
  License Version 1.0 (the "License"); you may not use this file except in
  compliance with the License. You may obtain a copy of the License here:
  http://www.flamerobin.org/license.html.

  Software distributed under the License is distributed on an "AS IS"
  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
  License for the specific language governing rights and limitations under
  the License.

  The Original Code is FlameRobin (TM).

  The Initial Developer of the Original Code is Milan Babuskov.

  Portions created by the original developer
  are Copyright (C) 2005 Milan Babuskov.

  All Rights Reserved.

  $Id: Identifier.h,v 1.7 2005/11/27 16:55:51 mghie Exp $

  Contributor(s):
*/
#ifndef FR_IDENTIFIER_H
#define FR_IDENTIFIER_H

#include <set>
//----------------------------------------------------------------------------
//! The purpose of this class is to abstract all the work with identifiers
//! so that we don't have to struggle with quoted identifiers all over the
//! place. If also makes matching easier (upper/lower case problems)
class Identifier
{
private:
    wxString textM;
    static bool isReserved(const wxString& s);
    static bool needsQuoting(const wxString& s);
    static bool isQuoted(const wxString &s);
    static wxString& escape(wxString& s);
    static wxString& strip(wxString& s);
    static wxString& quote(wxString &s);
public:
    typedef std::set<wxString> keywordContainer;
    Identifier();
    Identifier(const wxString& source);
    void setText(const wxString& source);
    void setFromSql(const wxString& source);

    static const keywordContainer& getKeywordSet();
    static wxString getKeywords(bool lowerCase = false);

    bool equals(const Identifier& rhs) const;
    bool equals(const wxString& rhs) const;
    wxString get() const;
    wxString getQuoted() const;
    static wxString userString(const wxString& s);
};
//----------------------------------------------------------------------------
#endif
