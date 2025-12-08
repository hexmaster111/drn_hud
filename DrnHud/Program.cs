
using System.Data.Common;
using System.Diagnostics;

DrnFile f = DrnFile.Parse(
    """
    A
        A1
        A2
        A3
    B
    C
        C1
        C2
        C3
            C3.1
            C3.2
            C3.3
    D
    """
);

PrintTree(f.StartOfFile);


return 0;

void PrintTree(Node n, int depth = 0)
{
    if (n == null) return;

    string indentStr = "";
    for (int i = 0; i < depth; i++) indentStr += ".";
    Console.WriteLine(indentStr + n.Code);

    if (n.Inner != null)
    {
        PrintTree(n.Inner, depth + 1);
    }

    PrintTree(n.Next, depth);
}



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