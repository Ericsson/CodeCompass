// $Id$
// Created by Aron Barath, 2014

package parser;

import java.util.List;

import org.eclipse.jdt.core.dom.Javadoc;
import org.eclipse.jdt.core.dom.MemberRef;
import org.eclipse.jdt.core.dom.MethodRef;
import org.eclipse.jdt.core.dom.Name;
import org.eclipse.jdt.core.dom.TagElement;
import org.eclipse.jdt.core.dom.TextElement;

public class HtmlBuilder
{
	public HtmlBuilder(Javadoc doc)
	{
		if(null==doc || null==doc.tags())
		{
			return;
		}

		enterDescription();
		for(Object o : doc.tags())
		{
			processAbstract(o);
		}
		leave();

		StringBuilder builder = new StringBuilder();
		builder.append("<div><dl>");
		appendIf(builder, builderDes, "Description");
		appendIf(builder, builderPar, "Parameters");
		appendIf(builder, builderRet, "Returns");
		appendIf(builder, builderThr, "Throws");
		appendIf(builder, builderExc, "Exception");
		appendIf(builder, builderSee, "See also");
		appendIf(builder, builderOth, "Other");
		builder.append("</dl></div>");
		html = builder.toString();
	}

	private StringBuilder builderDes = new StringBuilder();
	private StringBuilder builderPar = new StringBuilder();
	private StringBuilder builderRet = new StringBuilder();
	private StringBuilder builderThr = new StringBuilder();
	private StringBuilder builderExc = new StringBuilder();
	private StringBuilder builderSee = new StringBuilder();
	private StringBuilder builderOth = new StringBuilder();
	private StringBuilder actualBuilder = null;
	private java.util.List<StringBuilder> stack = new java.util.ArrayList<StringBuilder>();
	private void enterDescription() { stack.add(actualBuilder = builderDes); genericEnter(); }
	private void enterParameters() { stack.add(actualBuilder = builderPar); genericEnter(); }
	private void enterReturn() { stack.add(actualBuilder = builderRet); genericEnter(); }
	private void enterThrows() { stack.add(actualBuilder = builderThr); genericEnter(); }
	private void enterException() { stack.add(actualBuilder = builderExc); genericEnter(); }
	private void enterSeeAlso() { stack.add(actualBuilder = builderSee); genericEnter(); }

	private void enterOther(String tag)
	{
		stack.add(actualBuilder = builderOth);

		genericEnter();

		builderOth.append("<b>").append(tag).append("</b> ");
	}

	private void genericEnter()
	{
		if(0<actualBuilder.length())
		{
			actualBuilder.append("<br/>\n");
		}
	}

	private void leave()
	{
		stack.remove(stack.size() - 1);

		if(!stack.isEmpty())
		{
			actualBuilder = stack.get(stack.size() - 1);
		}
		else
		{
			actualBuilder = null;
		}
	}

	private String html = "";

	public boolean isEmpty() { return html.isEmpty(); }
	public String getHtml() { return html; }
	public long getHash() { return Utils.getDocHtmlHash(html); }

	private final String titleTag = "h2";

	private void appendIf(StringBuilder builder, StringBuilder source, String title)
	{
		if(0<source.length())
		{
			builder.append("<dt><").append(titleTag).append(">").append(title)
				.append("</").append(titleTag).append("></dt>\n<dd>")
				.append(source.toString()).append("</dd>\n");
		}
	}

	private void put(String s)
	{
		actualBuilder.append(s);
	}

	private void append(String s)
	{
		for(int i=0;i<s.length();++i)
		{
			char ch = s.charAt(i);

			switch(ch)
			{
			case '<':  actualBuilder.append("&lt;");  break;
			case '>':  actualBuilder.append("&gt;");  break;
			case '&':  actualBuilder.append("&amp;"); break;
			case '\n': actualBuilder.append("<br/>"); break;
			case '\r': /* ignore */                  break;
			default:   actualBuilder.append(ch);      break;
			}
		}
	}

	private void processAbstract(Object obj)
	{
		if(obj instanceof TagElement)
		{
			processTagElement((TagElement)obj);
		}
		else
		if(obj instanceof TextElement)
		{
			append(((TextElement)obj).getText());
		}
		else
		if(obj instanceof Name)
		{
			put("<b>");
			append(((Name)obj).getFullyQualifiedName());
			put("</b>");
		}
		else
		if(obj instanceof MemberRef)
		{
			put("<b>");
			append(((MemberRef)obj).getName().toString());
			put("</b>");
		}
		else
		if(obj instanceof MethodRef)
		{
			put("<b>");
			append(((MethodRef)obj).getName().toString());
			put("</b>");
		}
	}

	private void processTagElement(TagElement tag)
	{
		boolean pop = false;

		if(null!=tag.getTagName())
		{
			switch(tag.getTagName())
			{
			case TagElement.TAG_PARAM:      enterParameters();         pop = true; break;
			case TagElement.TAG_RETURN:     enterReturn();             pop = true; break;
			case TagElement.TAG_EXCEPTION:  enterException();          pop = true; break;
			case TagElement.TAG_THROWS:     enterThrows();             pop = true; break;
			case TagElement.TAG_SEE:        enterSeeAlso();            pop = true; break;
			case TagElement.TAG_AUTHOR:     enterOther("Author:");     pop = true; break;
			case TagElement.TAG_VERSION:    enterOther("Version:");    pop = true; break;
			case TagElement.TAG_DEPRECATED: enterOther("Deprecated:"); pop = true; break;
			case TagElement.TAG_SERIAL:     enterOther("Serial:");     pop = true; break;
			case TagElement.TAG_SINCE:      enterOther("Since:");      pop = true; break;
			}
		}

		for(Object o : tag.fragments())
		{
			processAbstract(o);
		}

		if(pop)
		{
			leave();
		}
	}
}
