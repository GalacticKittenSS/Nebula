﻿namespace Nebula
{
    public class Font : Asset
    {
        protected Font()
            : base()
        {
        }

        internal Font(ulong handle)
            : base(handle)
        {
        }

        public Font(string path)
            : base(path)
        {
        }

        public bool Bold
        {
            get => InternalCalls.Font_GetBold(ID);
        }

        public bool Italic
        {
            get => InternalCalls.Font_GetItalic(ID);
        }
        
        public static implicit operator string(Font font) => font.FilePath;
        public static implicit operator Font(string path) => new Font(path);
    }
}
