/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 MusicXML support.
 */

#include "global/serialization/xmlstreamreader.h"

#include "translation.h"
#include "engraving/dom/articulation.h"
#include "engraving/dom/chord.h"
#include "types/symnames.h"
#include "musicxmlsupport.h"

namespace mu::iex::musicxml {
//---------------------------------------------------------
//   errorStringWithLocation
//---------------------------------------------------------

String errorStringWithLocation(int line, int col, const String& error)
{
    return muse::mtrc("iex_musicxml", "line %1, column %2:").arg(line).arg(col) + u" " + error;
}

//---------------------------------------------------------
//   checkAtEndElement
//---------------------------------------------------------

muse::String checkAtEndElement(const muse::XmlStreamReader& e, const muse::String& expName)
{
    if (e.isEndElement() && e.name() == expName.toAscii().constChar()) {
        return String();
    }

    String res = muse::mtrc("iex_musicxml", "expected token type and name ‘EndElement %1’, actual ‘%2 %3’")
                 .arg(expName)
                 .arg(muse::String::fromAscii(e.tokenString().ascii()))
                 .arg(muse::String::fromAscii(e.name().ascii()));
    return res;
}

//---------------------------------------------------------
//   stringToInt
//---------------------------------------------------------

/**
 Convert a string in \a s into an int. Set *ok to true iff conversion was
 successful. \a s may end with ".0", as is generated by Audiveris 3.2 and up,
 in elements <divisions>, <duration>, <alter> and <sound> attributes
 dynamics and tempo.
 In case of error val return a default value of 0.
 Note that non-integer values cannot be handled by mscore.
 */

int MusicXmlSupport::stringToInt(const String& s, bool* ok)
{
    int res = 0;
    String str = s;
    if (s.endsWith(u".0")) {
        str = s.left(s.size() - 2);
    }
    res = str.toInt(ok);
    return res;
}

//---------------------------------------------------------
//   noteTypeToFraction
//---------------------------------------------------------

/**
 Convert MusicXML note type to fraction.
 */

Fraction MusicXmlSupport::noteTypeToFraction(const String& type)
{
    if (type == u"1024th") {
        return Fraction(1, 1024);
    } else if (type == u"512th") {
        return Fraction(1, 512);
    } else if (type == u"256th") {
        return Fraction(1, 256);
    } else if (type == u"128th") {
        return Fraction(1, 128);
    } else if (type == u"64th") {
        return Fraction(1, 64);
    } else if (type == u"32nd") {
        return Fraction(1, 32);
    } else if (type == u"16th") {
        return Fraction(1, 16);
    } else if (type == u"eighth") {
        return Fraction(1, 8);
    } else if (type == u"quarter") {
        return Fraction(1, 4);
    } else if (type == u"half") {
        return Fraction(1, 2);
    } else if (type == u"whole") {
        return Fraction(1, 1);
    } else if (type == u"breve") {
        return Fraction(2, 1);
    } else if (type == u"long") {
        return Fraction(4, 1);
    } else if (type == u"maxima") {
        return Fraction(8, 1);
    } else {
        return Fraction(0, 0);
    }
}

//---------------------------------------------------------
//   calculateFraction
//---------------------------------------------------------

/**
 Convert note type, number of dots and actual and normal notes into a duration
 */

Fraction MusicXmlSupport::calculateFraction(const String& type, int dots, int normalNotes, int actualNotes)
{
    // type
    Fraction f = MusicXmlSupport::noteTypeToFraction(type);
    if (f.isValid()) {
        // dot(s)
        Fraction f_no_dots = f;
        for (int i = 0; i < dots; ++i) {
            f += (f_no_dots / Fraction(2 << i, 1));
        }
        // tuplet
        if (actualNotes > 0 && normalNotes > 0) {
            f *= normalNotes;
            f /= Fraction(actualNotes, 1);
        }
        // clean up (just in case)
        f.reduce();
    }
    return f;
}

//---------------------------------------------------------
//   accSymId2MusicXmlString
//---------------------------------------------------------

String accSymId2MusicXmlString(const SymId id)
{
    String s;
    switch (id) {
    case SymId::accidentalSharp:                 s = u"sharp";
        break;
    case SymId::accidentalNatural:               s = u"natural";
        break;
    case SymId::accidentalFlat:                  s = u"flat";
        break;
    case SymId::accidentalDoubleSharp:           s = u"double-sharp";
        break;
    //case SymId::accidentalDoubleSharp:           s = u"sharp-sharp"; break; // see above
    //case SymId::accidentalDoubleFlat:            s = u"double-flat"; break; // doesn't exist in MusicXML, but see below
    case SymId::accidentalDoubleFlat:            s = u"flat-flat";
        break;
    case SymId::accidentalNaturalSharp:          s = u"natural-sharp";
        break;
    case SymId::accidentalNaturalFlat:           s = u"natural-flat";
        break;
    case SymId::accidentalQuarterToneFlatStein:  s = u"quarter-flat";
        break;
    case SymId::accidentalQuarterToneSharpStein: s = u"quarter-sharp";
        break;
    case SymId::accidentalThreeQuarterTonesFlatZimmermann: s = u"three-quarters-flat";
        break;
    //case SymId::noSym:                                     s = u"three-quarters-flat";  break; // AccidentalType::FLAT_FLAT_SLASH, MuseScore 1?
    case SymId::accidentalThreeQuarterTonesSharpStein:     s = u"three-quarters-sharp";
        break;
    case SymId::accidentalQuarterToneSharpArrowDown:       s = u"sharp-down";
        break;
    case SymId::accidentalThreeQuarterTonesSharpArrowUp:   s = u"sharp-up";
        break;
    case SymId::accidentalQuarterToneFlatNaturalArrowDown: s = u"natural-down";
        break;
    case SymId::accidentalQuarterToneSharpNaturalArrowUp:  s = u"natural-up";
        break;
    case SymId::accidentalThreeQuarterTonesFlatArrowDown:  s = u"flat-down";
        break;
    case SymId::accidentalQuarterToneFlatArrowUp:          s = u"flat-up";
        break;
    case SymId::accidentalThreeQuarterTonesSharpArrowDown: s = u"double-sharp-down";
        break;
    case SymId::accidentalFiveQuarterTonesSharpArrowUp:    s = u"double-sharp-up";
        break;
    case SymId::accidentalFiveQuarterTonesFlatArrowDown:   s = u"flat-flat-down";
        break;
    case SymId::accidentalThreeQuarterTonesFlatArrowUp:    s = u"flat-flat-up";
        break;

    case SymId::accidentalArrowDown:             s = u"arrow-down";
        break;
    case SymId::accidentalArrowUp:               s = u"arrow-up";
        break;

    case SymId::accidentalTripleSharp:           s = u"triple-sharp";
        break;
    case SymId::accidentalTripleFlat:            s = u"triple-flat";
        break;

    case SymId::accidentalKucukMucennebSharp:    s = u"slash-quarter-sharp";
        break;
    case SymId::accidentalBuyukMucennebSharp:    s = u"slash-sharp";
        break;
    case SymId::accidentalBakiyeFlat:            s = u"slash-flat";
        break;
    case SymId::accidentalBuyukMucennebFlat:     s = u"double-slash-flat";
        break;

    case SymId::accidental1CommaSharp:           s = u"sharp-1";
        break;
    case SymId::accidental2CommaSharp:           s = u"sharp-2";
        break;
    case SymId::accidental3CommaSharp:           s = u"sharp-3";
        break;
    case SymId::accidental5CommaSharp:           s = u"sharp-5";
        break;
    case SymId::accidental1CommaFlat:            s = u"flat-1";
        break;
    case SymId::accidental2CommaFlat:            s = u"flat-2";
        break;
    case SymId::accidental3CommaFlat:            s = u"flat-3";
        break;
    case SymId::accidental4CommaFlat:            s = u"flat-4";
        break;

    case SymId::accidentalSori:                  s = u"sori";
        break;
    case SymId::accidentalKoron:                 s = u"koron";
        break;
    default:
        s = u"other";
        LOGD("accSymId2MusicXmlString: unknown accidental %d", static_cast<int>(id));
    }
    return s;
}

//---------------------------------------------------------
//   accSymId2SmuflMusicXmlString
//---------------------------------------------------------

String accSymId2SmuflMusicXmlString(const SymId id)
{
    return String::fromAscii(SymNames::nameForSymId(id).ascii());
}

//---------------------------------------------------------
//   musicXmlString2accSymId
// see https://github.com/w3c/musicxml/blob/6e3a667b85855b04d7e4548ea508b537bc29fc52/schema/musicxml.xsd#L1392-L1439
//---------------------------------------------------------

SymId musicXmlString2accSymId(const String mxmlName, const String smufl)
{
    // map MusicXML accidental name to MuseScore enum SymId
    static const std::map<String, SymId> map {
        { u"sharp", SymId::accidentalSharp },
        { u"natural", SymId::accidentalNatural },
        { u"flat", SymId::accidentalFlat },
        { u"double-sharp", SymId::accidentalDoubleSharp },
        { u"sharp-sharp", SymId::accidentalDoubleSharp },
        //{ u"double-flat", SymId::accidentalDoubleFlat }, // shouldn't harm, but doesn't exist in MusicXML
        { u"flat-flat", SymId::accidentalDoubleFlat },
        { u"natural-sharp", SymId::accidentalNaturalSharp },
        { u"natural-flat", SymId::accidentalNaturalFlat },

        { u"quarter-flat", SymId::accidentalQuarterToneFlatStein },
        { u"quarter-sharp", SymId::accidentalQuarterToneSharpStein },
        { u"three-quarters-flat", SymId::accidentalThreeQuarterTonesFlatZimmermann },
        { u"three-quarters-sharp", SymId::accidentalThreeQuarterTonesSharpStein },

        { u"sharp-down", SymId::accidentalQuarterToneSharpArrowDown },
        { u"sharp-up", SymId::accidentalThreeQuarterTonesSharpArrowUp },
        { u"natural-down", SymId::accidentalQuarterToneFlatNaturalArrowDown },
        { u"natural-up", SymId::accidentalQuarterToneSharpNaturalArrowUp },
        { u"flat-down", SymId::accidentalThreeQuarterTonesFlatArrowDown },
        { u"flat-up", SymId::accidentalQuarterToneFlatArrowUp },
        { u"double-sharp-down", SymId::accidentalThreeQuarterTonesSharpArrowDown },
        { u"double-sharp-up", SymId::accidentalFiveQuarterTonesSharpArrowUp },
        { u"flat-flat-down", SymId::accidentalFiveQuarterTonesFlatArrowDown },
        { u"flat-flat-up", SymId::accidentalThreeQuarterTonesFlatArrowUp },

        { u"arrow-down", SymId::accidentalArrowDown },
        { u"arrow-up", SymId::accidentalArrowUp },

        { u"triple-sharp", SymId::accidentalTripleSharp },
        { u"triple-flat", SymId::accidentalTripleFlat },

        { u"slash-quarter-sharp", SymId::accidentalKucukMucennebSharp },
        { u"slash-sharp", SymId::accidentalBuyukMucennebSharp },
        { u"slash-flat", SymId::accidentalBakiyeFlat },
        { u"double-slash-flat", SymId::accidentalBuyukMucennebFlat },

        { u"sharp-1", SymId::accidental1CommaSharp },
        { u"sharp-2", SymId::accidental2CommaSharp },
        { u"sharp-3", SymId::accidental3CommaSharp },
        { u"sharp-5", SymId::accidental5CommaSharp },
        { u"flat-1", SymId::accidental1CommaFlat },
        { u"flat-2", SymId::accidental2CommaFlat },
        { u"flat-3", SymId::accidental3CommaFlat },
        { u"flat-4", SymId::accidental4CommaFlat },

        { u"sori", SymId::accidentalSori },
        { u"koron", SymId::accidentalKoron },
    };

    auto it = map.find(mxmlName);
    if (it != map.end()) {
        return it->second;
    } else if (mxmlName == u"other") {
        return SymNames::symIdByName(smufl);
    } else {
        LOGD("musicXmlString2accSymId: unknown accidental '%s'", muPrintable(mxmlName));
    }

    // default
    return SymId::noSym;
}

//---------------------------------------------------------
//   accidentalType2MusicXmlString
//---------------------------------------------------------

String accidentalType2MusicXmlString(const AccidentalType type)
{
    String s;
    switch (type) {
    case AccidentalType::SHARP:              s = u"sharp";
        break;
    case AccidentalType::NATURAL:            s = u"natural";
        break;
    case AccidentalType::FLAT:               s = u"flat";
        break;
    case AccidentalType::SHARP2:             s = u"double-sharp";
        break;
    //case AccidentalType::SHARP2:             s = u"sharp-sharp"; break; // see above
    //case AccidentalType::FLAT2:              s = u"double-flat"; break; // doesn't exist in MusicXML, but see below
    case AccidentalType::FLAT2:              s = u"flat-flat";
        break;
    case AccidentalType::NATURAL_SHARP:      s = u"natural-sharp";
        break;
    case AccidentalType::NATURAL_FLAT:       s = u"natural-flat";
        break;
    case AccidentalType::SHARP_ARROW_UP:     s = u"sharp-up";
        break;
    case AccidentalType::MIRRORED_FLAT:      s = u"quarter-flat";
        break;
    case AccidentalType::SHARP_SLASH:        s = u"quarter-sharp";
        break;
    case AccidentalType::MIRRORED_FLAT2:     s = u"three-quarters-flat";
        break;
    //case AccidentalType::FLAT_FLAT_SLASH:    s = u"three-quarters-flat";  break; // MuseScore 1?
    case AccidentalType::SHARP_SLASH4:       s = u"three-quarters-sharp";
        break;
    case AccidentalType::SHARP_ARROW_DOWN:   s = u"sharp-down";
        break;
    case AccidentalType::NATURAL_ARROW_UP:   s = u"natural-up";
        break;
    case AccidentalType::NATURAL_ARROW_DOWN: s = u"natural-down";
        break;
    case AccidentalType::FLAT_ARROW_DOWN:    s = u"flat-down";
        break;
    case AccidentalType::FLAT_ARROW_UP:      s = u"flat-up";
        break;
    case AccidentalType::SHARP2_ARROW_DOWN:  s = u"double-sharp-down";
        break;
    case AccidentalType::SHARP2_ARROW_UP:    s = u"double-sharp-up";
        break;
    case AccidentalType::FLAT2_ARROW_DOWN:   s = u"flat-flat-down";
        break;
    case AccidentalType::FLAT2_ARROW_UP:     s = u"flat-flat-up";
        break;

    case AccidentalType::ARROW_DOWN:         s = u"arrow-down";
        break;
    case AccidentalType::ARROW_UP:           s = u"arrow-up";
        break;

    case AccidentalType::SHARP3:             s = u"triple-sharp";
        break;
    case AccidentalType::FLAT3:              s = u"triple-flat";
        break;

    case AccidentalType::SHARP_SLASH3:       s = u"slash-quarter-sharp";
        break;
    case AccidentalType::SHARP_SLASH2:       s = u"slash-sharp";
        break;
    case AccidentalType::FLAT_SLASH:         s = u"slash-flat";
        break;
    case AccidentalType::FLAT_SLASH2:        s = u"double-slash-flat";
        break;

    case AccidentalType::ONE_COMMA_SHARP:    s = u"sharp-1";
        break;
    case AccidentalType::TWO_COMMA_SHARP:    s = u"sharp-2";
        break;
    case AccidentalType::THREE_COMMA_SHARP:  s = u"sharp-3";
        break;
    case AccidentalType::FIVE_COMMA_SHARP:   s = u"sharp-5";
        break;
    case AccidentalType::ONE_COMMA_FLAT:     s = u"flat-1";
        break;
    case AccidentalType::TWO_COMMA_FLAT:     s = u"flat-2";
        break;
    case AccidentalType::THREE_COMMA_FLAT:   s = u"flat-3";
        break;
    case AccidentalType::FOUR_COMMA_FLAT:    s = u"flat-4";
        break;

    case AccidentalType::SORI:               s = u"sori";
        break;
    case AccidentalType::KORON:              s = u"koron";
        break;
    default:
        s = u"other";
    }
    return s;
}

static const std::map<muse::String, AccidentalType> smuflAccidentalTypes {
    { u"accidentalDoubleFlatOneArrowDown",                AccidentalType::DOUBLE_FLAT_ONE_ARROW_DOWN },
    { u"accidentalFlatOneArrowDown",                      AccidentalType::FLAT_ONE_ARROW_DOWN },
    { u"accidentalNaturalOneArrowDown",                   AccidentalType::NATURAL_ONE_ARROW_DOWN },
    { u"accidentalSharpOneArrowDown",                     AccidentalType::SHARP_ONE_ARROW_DOWN },
    { u"accidentalDoubleSharpOneArrowDown",               AccidentalType::DOUBLE_SHARP_ONE_ARROW_DOWN },
    { u"accidentalDoubleFlatOneArrowUp",                  AccidentalType::DOUBLE_FLAT_ONE_ARROW_UP },
    { u"accidentalFlatOneArrowUp",                        AccidentalType::FLAT_ONE_ARROW_UP },
    { u"accidentalNaturalOneArrowUp",                     AccidentalType::NATURAL_ONE_ARROW_UP },
    { u"accidentalSharpOneArrowUp",                       AccidentalType::SHARP_ONE_ARROW_UP },
    { u"accidentalDoubleSharpOneArrowUp",                 AccidentalType::DOUBLE_SHARP_ONE_ARROW_UP },
    { u"accidentalDoubleFlatTwoArrowsDown",               AccidentalType::DOUBLE_FLAT_TWO_ARROWS_DOWN },
    { u"accidentalFlatTwoArrowsDown",                     AccidentalType::FLAT_TWO_ARROWS_DOWN },
    { u"accidentalNaturalTwoArrowsDown",                  AccidentalType::NATURAL_TWO_ARROWS_DOWN },
    { u"accidentalSharpTwoArrowsDown",                    AccidentalType::SHARP_TWO_ARROWS_DOWN },
    { u"accidentalDoubleSharpTwoArrowsDown",              AccidentalType::DOUBLE_SHARP_TWO_ARROWS_DOWN },
    { u"accidentalDoubleFlatTwoArrowsUp",                 AccidentalType::DOUBLE_FLAT_TWO_ARROWS_UP },
    { u"accidentalFlatTwoArrowsUp",                       AccidentalType::FLAT_TWO_ARROWS_UP },
    { u"accidentalNaturalTwoArrowsUp",                    AccidentalType::NATURAL_TWO_ARROWS_UP },
    { u"accidentalSharpTwoArrowsUp",                      AccidentalType::SHARP_TWO_ARROWS_UP },
    { u"accidentalDoubleSharpTwoArrowsUp",                AccidentalType::DOUBLE_SHARP_TWO_ARROWS_UP },
    { u"accidentalDoubleFlatThreeArrowsDown",             AccidentalType::DOUBLE_FLAT_THREE_ARROWS_DOWN },
    { u"accidentalFlatThreeArrowsDown",                   AccidentalType::FLAT_THREE_ARROWS_DOWN },
    { u"accidentalNaturalThreeArrowsDown",                AccidentalType::NATURAL_THREE_ARROWS_DOWN },
    { u"accidentalSharpThreeArrowsDown",                  AccidentalType::SHARP_THREE_ARROWS_DOWN },
    { u"accidentalDoubleSharpThreeArrowsDown",            AccidentalType::DOUBLE_SHARP_THREE_ARROWS_DOWN },
    { u"accidentalDoubleFlatThreeArrowsUp",               AccidentalType::DOUBLE_FLAT_THREE_ARROWS_UP },
    { u"accidentalFlatThreeArrowsUp",                     AccidentalType::FLAT_THREE_ARROWS_UP },
    { u"accidentalNaturalThreeArrowsUp",                  AccidentalType::NATURAL_THREE_ARROWS_UP },
    { u"accidentalSharpThreeArrowsUp",                    AccidentalType::SHARP_THREE_ARROWS_UP },
    { u"accidentalDoubleSharpThreeArrowsUp",              AccidentalType::DOUBLE_SHARP_THREE_ARROWS_UP },
    { u"accidentalLowerOneSeptimalComma",                 AccidentalType::LOWER_ONE_SEPTIMAL_COMMA },
    { u"accidentalRaiseOneSeptimalComma",                 AccidentalType::RAISE_ONE_SEPTIMAL_COMMA },
    { u"accidentalLowerTwoSeptimalCommas",                AccidentalType::LOWER_TWO_SEPTIMAL_COMMAS },
    { u"accidentalRaiseTwoSeptimalCommas",                AccidentalType::RAISE_TWO_SEPTIMAL_COMMAS },
    { u"accidentalLowerOneUndecimalQuartertone",          AccidentalType::LOWER_ONE_UNDECIMAL_QUARTERTONE },
    { u"accidentalRaiseOneUndecimalQuartertone",          AccidentalType::RAISE_ONE_UNDECIMAL_QUARTERTONE },
    { u"accidentalLowerOneTridecimalQuartertone",         AccidentalType::LOWER_ONE_TRIDECIMAL_QUARTERTONE },
    { u"accidentalRaiseOneTridecimalQuartertone",         AccidentalType::RAISE_ONE_TRIDECIMAL_QUARTERTONE },
    { u"accidentalDoubleFlatEqualTempered",               AccidentalType::DOUBLE_FLAT_EQUAL_TEMPERED },
    { u"accidentalFlatEqualTempered",                     AccidentalType::FLAT_EQUAL_TEMPERED },
    { u"accidentalNaturalEqualTempered",                  AccidentalType::NATURAL_EQUAL_TEMPERED },
    { u"accidentalSharpEqualTempered",                    AccidentalType::SHARP_EQUAL_TEMPERED },
    { u"accidentalDoubleSharpEqualTempered",              AccidentalType::DOUBLE_SHARP_EQUAL_TEMPERED },
    { u"accidentalQuarterFlatEqualTempered",              AccidentalType::QUARTER_FLAT_EQUAL_TEMPERED },
    { u"accidentalQuarterSharpEqualTempered",             AccidentalType::QUARTER_SHARP_EQUAL_TEMPERED }
};

//---------------------------------------------------------
//   accidentalType2SmuflMusicXmlString
//---------------------------------------------------------

String accidentalType2SmuflMusicXmlString(const AccidentalType type)
{
    return muse::key(smuflAccidentalTypes, type);
}

//---------------------------------------------------------
//   musicXmlString2accidentalType
//---------------------------------------------------------

/**
 Convert a MusicXML accidental name to a MuseScore enum AccidentalType.
 see https://github.com/w3c/musicxml/blob/6e3a667b85855b04d7e4548ea508b537bc29fc52/schema/musicxml.xsd#L1392-L1439
 */

AccidentalType musicXmlString2accidentalType(const String mxmlName, const String smufl)
{
    // map MusicXML accidental name to MuseScore enum AccidentalType
    static const std::map<String, AccidentalType> map {
        { u"sharp", AccidentalType::SHARP },
        { u"natural", AccidentalType::NATURAL },
        { u"flat", AccidentalType::FLAT },
        { u"double-sharp", AccidentalType::SHARP2 },
        { u"sharp-sharp", AccidentalType::SHARP2 },
        //{ u"double-flat", AccidentalType::FLAT2 }, // shouldn't harm, but doesn't exist in MusicXML
        { u"flat-flat", AccidentalType::FLAT2 },
        { u"natural-sharp", AccidentalType::SHARP },
        { u"natural-flat", AccidentalType::FLAT },

        { u"quarter-flat", AccidentalType::MIRRORED_FLAT },
        { u"quarter-sharp", AccidentalType::SHARP_SLASH },
        { u"three-quarters-flat", AccidentalType::MIRRORED_FLAT2 },
        { u"three-quarters-sharp", AccidentalType::SHARP_SLASH4 },

        { u"sharp-up", AccidentalType::SHARP_ARROW_UP },
        { u"natural-down", AccidentalType::NATURAL_ARROW_DOWN },
        { u"natural-up", AccidentalType::NATURAL_ARROW_UP },
        { u"sharp-down", AccidentalType::SHARP_ARROW_DOWN },
        { u"flat-down", AccidentalType::FLAT_ARROW_DOWN },
        { u"flat-up", AccidentalType::FLAT_ARROW_UP },
        { u"double-sharp-down", AccidentalType::SHARP2_ARROW_DOWN },
        { u"double-sharp-up", AccidentalType::SHARP2_ARROW_UP },
        { u"flat-flat-down", AccidentalType::FLAT2_ARROW_DOWN },
        { u"flat-flat-up", AccidentalType::FLAT2_ARROW_UP },

        { u"arrow-down", AccidentalType::ARROW_DOWN },
        { u"arrow-up", AccidentalType::ARROW_UP },

        { u"triple-sharp", AccidentalType::SHARP3 },
        { u"triple-flat", AccidentalType::FLAT3 },

        { u"slash-quarter-sharp", AccidentalType::SHARP_SLASH3 }, // MIRRORED_FLAT_SLASH }, ?
        { u"slash-sharp", AccidentalType::SHARP_SLASH2 }, // SHARP_SLASH }, ?
        { u"slash-flat", AccidentalType::FLAT_SLASH },
        { u"double-slash-flat", AccidentalType::FLAT_SLASH2 },

        { u"sharp-1", AccidentalType::ONE_COMMA_SHARP },
        { u"sharp-2", AccidentalType::TWO_COMMA_SHARP },
        { u"sharp-3", AccidentalType::THREE_COMMA_SHARP },
        { u"sharp-5", AccidentalType::FIVE_COMMA_SHARP },
        { u"flat-1", AccidentalType::ONE_COMMA_FLAT },
        { u"flat-2", AccidentalType::TWO_COMMA_FLAT },
        { u"flat-3", AccidentalType::THREE_COMMA_FLAT },
        { u"flat-4", AccidentalType::FOUR_COMMA_FLAT },

        { u"sori", AccidentalType::SORI },
        { u"koron", AccidentalType::KORON },
    };

    auto it = map.find(mxmlName);
    if (it != map.end()) {
        return it->second;
    } else if (mxmlName == "other" && muse::contains(smuflAccidentalTypes, smufl)) {
        return smuflAccidentalTypes.at(smufl);
    } else {
        LOGD("musicXmlString2accidentalType: unknown accidental '%s'", muPrintable(mxmlName));
    }
    return AccidentalType::NONE;
}

//---------------------------------------------------------
//   musicXmlAccidentalTextToChar
//---------------------------------------------------------

/**
 Convert a MusicXML accidental text to a accidental character.
 */

String musicXmlAccidentalTextToChar(const String mxmlName)
{
    // map MusicXML accidental name to MuseScore enum AccidentalType
    static const std::map<String, String> map {
        { u"sharp", u"♯" },
        { u"natural", u"♮" },
        { u"flat", u"♭" },
    };

    auto it = map.find(mxmlName);
    if (it != map.end()) {
        return it->second;
    } else {
        LOGD("musicXmlAccidentalTextToChar: unsupported accidental '%s'", muPrintable(mxmlName));
    }
    return u"";
}

//---------------------------------------------------------
//   isAppr
//---------------------------------------------------------

/**
 Check if v approximately equals ref.
 Used to prevent floating point comparison for equality from failing
 */

static bool isAppr(const double v, const double ref, const double epsilon)
{
    return v > ref - epsilon && v < ref + epsilon;
}

//---------------------------------------------------------
//   microtonalGuess
//---------------------------------------------------------

/**
 Convert a MusicXML alter tag into a microtonal accidental in MuseScore enum AccidentalType.
 Works only for quarter tone, half tone, three-quarters tone and whole tone accidentals.
 */

AccidentalType microtonalGuess(double val)
{
    const double eps = 0.001;
    if (isAppr(val, -2, eps)) {
        return AccidentalType::FLAT2;
    } else if (isAppr(val, -1.5, eps)) {
        return AccidentalType::MIRRORED_FLAT2;
    } else if (isAppr(val, -1, eps)) {
        return AccidentalType::FLAT;
    } else if (isAppr(val, -0.5, eps)) {
        return AccidentalType::MIRRORED_FLAT;
    } else if (isAppr(val, 0, eps)) {
        return AccidentalType::NATURAL;
    } else if (isAppr(val, 0.5, eps)) {
        return AccidentalType::SHARP_SLASH;
    } else if (isAppr(val, 1, eps)) {
        return AccidentalType::SHARP;
    } else if (isAppr(val, 1.5, eps)) {
        return AccidentalType::SHARP_SLASH4;
    } else if (isAppr(val, 2, eps)) {
        return AccidentalType::SHARP2;
    } else {
        LOGD("Guess for microtonal accidental corresponding to value %f failed.", val);
    }
    // default
    return AccidentalType::NONE;
}

//---------------------------------------------------------
//   isLaissezVibrer
//---------------------------------------------------------

bool isLaissezVibrer(const SymId id)
{
    return id == SymId::articLaissezVibrerAbove || id == SymId::articLaissezVibrerBelow;
}

//---------------------------------------------------------
//   findLaissezVibrer
//---------------------------------------------------------

// TODO: there should be a lambda hiding somewhere ...

const Articulation* findLaissezVibrer(const Chord* chord)
{
    for (const Articulation* a : chord->articulations()) {
        if (isLaissezVibrer(a->symId())) {
            return a;
        }
    }
    return nullptr;
}
}
