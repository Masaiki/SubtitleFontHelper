using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

using libSubtitleFontHelper;

namespace SubtitleFontHelper
{
    [XmlRoot("FontFileCollection")]
    public class FontFileCollection
    {
        [XmlElement("FileCount")]
        public int FileCount { 
            get { return FontFiles.Count; }
            set { /*Eat the value*/ }
        }

        [XmlElement("FontFiles")]
        public List<FontFile> FontFiles { get; set; } = new List<FontFile>();
    }

    public class FontFile
    {
        [XmlElement("FileName")]
        public string FileName { get; set; }

        [XmlElement("FontFace")]
        public List<FontFace> FontFaces { get; set; } = new List<FontFace>();
    }

    public class FontFace
    {
        [XmlAttribute("index")]
        public int FontFaceIndex { get; set; } = 0;

        [XmlElement("PSName")]
        public List<string> PostScriptNames { get; set; }
        [XmlElement("FullName")]
        public List<string> FullNames { get; set; }
        [XmlElement("FamilyName")]
        public List<string> FamilyNames { get; set; }

        public static FontFace FromMetadata(IFreeTypeFontFaceMetadata metadata)
        {
            FontFace font = new FontFace();
            font.PostScriptNames = new List<string>(metadata.GetPostScriptNames());
            font.FullNames = new List<string>(metadata.GetFullNames());
            font.FamilyNames = new List<string>(metadata.GetFamilyNames());
            return font;
        }
    }
}
