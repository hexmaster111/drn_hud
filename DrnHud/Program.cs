
using System.Data.Common;
using System.Diagnostics;

DrnFile f = DrnFile.Parse(
    """
    # Comment blah blah blah
    TASK: T0
    CONDITION: C0
        TASK: T1
    TASK: T2
    """
);

return 0;




[DebuggerDisplay("{Code}")]
public class Node
{
    public static readonly Node EndOfFile = new(new() { Code = "[EOF]", Indent = 0 });

    public Node(Token tok)
    {
        Token = tok;
    }

    public Token Token;
    public string Code => Token.Code;
    public int Indent => Token.Indent;


    public Node Parent;
    public Node Next;
    public Node? Inner;
}



public class Inserter
{
    public Inserter(Node node) => Node = node;

    public Node Node;
    public int Indent = 0;

    public void Insert(Token tk)
    {
        var newNode = new Node(tk);

        if (newNode.Indent == Indent)
        {
            Node.Next = newNode;
            newNode.Parent = Node;
        }
        else if (newNode.Indent > Indent)
        {
            Node.Inner = newNode;
            newNode.Parent = Node;
            Indent = newNode.Indent;
        }
        else if (newNode.Indent < Indent)
        {
            var parent = Node.Parent;

            while (parent.Token.Indent != newNode.Indent)
            {
                parent = parent.Parent;
            }

            parent.Next = newNode;
            newNode.Parent = parent;

            Node.Next = newNode;

            Indent = newNode.Indent;
        }

        Node = newNode;
    }
}

public class DrnFile
{
    public Node StartOfFile = Node.EndOfFile;

    public static DrnFile Parse(string fileText, int spacesToIndent = 4)
    {
        Queue<Token> linesToParse = new(fileText.Split(['\r', '\n'], StringSplitOptions.RemoveEmptyEntries).Select(x => Token.FromLine(x, spacesToIndent)));

        var ret = new DrnFile();

        ret.StartOfFile = new Node(linesToParse.Dequeue());

        var inserter = new Inserter(ret.StartOfFile);

        while (linesToParse.Count != 0)
        {
            inserter.Insert(linesToParse.Dequeue());
        }

        inserter.Node.Next = Node.EndOfFile;

        return ret;
    }
}