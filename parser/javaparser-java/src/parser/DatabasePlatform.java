// $Id$
// Created by Aron Barath, 2013

package parser;

public class DatabasePlatform extends org.eclipse.persistence.platform.database.DatabasePlatform
{
	private static final long serialVersionUID = 7977396684032746594L;

	public boolean supportsForeignKeyConstraints()
	{
        return false;
	}
}
