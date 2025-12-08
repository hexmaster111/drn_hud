

using System.Diagnostics;

[DebuggerDisplay("{Indent} {Code}")]
public struct Token
{
    public int Indent;
    public string Code;

    public static Token FromLine(string line, int spacesToIndent)
    {
        int lineIdent;
        string code = line.TrimStart();
        string spaces = "";
        int spacesCount = 0;

        for (int i = 0; i < spacesToIndent; i++) spaces += " ";
        line = line.Replace("\t", spaces);

        foreach (var item in line)
        {
            if (item == ' ') spacesCount++;
            else break;
        }

        lineIdent = spacesCount / spacesToIndent;

        if (spacesCount % spacesToIndent != 0)
        {
            Console.WriteLine($"Warning: {line} indent is not consistant with spaces to ident");
        }

        return new() { Code = code, Indent = lineIdent };
    }
}
