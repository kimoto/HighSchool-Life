import java.io.*;
import java.util.*;

class OthelloException extends Exception
{
	public OthelloException (String str)
	{
		super( str );
	}
}

class Stone implements Cloneable
{
	public static final int EMPTY = 0;
	public static final int BLACK = 1;
	public static final int WHITE = 2;

	private int status_ = EMPTY;

	public Stone (int status)
	{
		if( status != EMPTY && status != BLACK
				&& status != WHITE )
		{
			throw new IllegalArgumentException("don't know status value");
		}
		this.status_ = status;
	}

	public boolean isEmpty ()
	{
		return ( status_ == EMPTY );
	}

	public boolean isBlack ()
	{
		return ( status_ == BLACK );
	}

	public boolean isWhite ()
	{
		return ( status_ == WHITE );
	}

	public void toBlack ()
	{
		status_ = BLACK;
	}

	public void toWhite ()
	{
		status_ = WHITE;
	}

	public void reverse ()
	{
		if( isWhite() )
			toBlack();
		else if( isBlack() )
			toWhite();
	}

	public boolean equals (Object obj)
	{
		Stone stone = (Stone)obj;

		if( this.isEmpty() || stone.isEmpty() )
			return false;

		if( this.isWhite() && stone.isWhite() )
			return true;

		if( this.isBlack() && stone.isBlack() )
			return true;

		return false;
	}

	public Object clone ()
	{
		Stone newStone = new Stone( Stone.EMPTY );

		if( isBlack() )
		{
			newStone.toBlack();
		}
		else if( isWhite() )
		{
			newStone.toWhite();
		}
		return newStone;
	}
}

class Field implements Cloneable
{
	private final Stone[][] field_;
	private final int xLength_;
	private final int yLength_;

	public Field (int xSize, int ySize)
	{
		this.xLength_ = xSize;
		this.yLength_ = ySize;
		
		field_ = new Stone[ xLength_ ][ yLength_ ];

		for( int x = 0; x < xLength_; x++ )
		{
			for( int y = 0; y < yLength_; y++ )
			{
				field_[ x ][ y ] = new Stone( Stone.EMPTY );
			}
		}

		Stone blackStone = new Stone( Stone.BLACK );
		Stone whiteStone = new Stone( Stone.WHITE );

		int xx = 5/2;
		int yy = 5/2;

		field_[xx][yy] = blackStone;
		field_[xx][yy-1] = whiteStone;
		field_[xx-1][yy-1] = blackStone;
		field_[xx-1][yy] = whiteStone;
	}

	public void setStone (int x, int y, Stone stone)
		throws OthelloException
	{
		if( field_[x][y].isEmpty() )
		{
			field_[x][y] = (Stone)stone.clone();

			reverseUpper(x, y, stone, false);
			//reverseLower(x, y, stone, false);
			//reverseLeft(x, y, stone, false);
			//reverseRight(x, y, stone, false);
		
			//reverseUpperRight(x, y, stone, false);
			//reverseUpperLeft(x, y, stone, false);
			//reverseLowerRight(x, y, stone, false);
			//reverseLowerLeft(x, y, stone, false);
		}
		else
		{
			throw new OthelloException("already exist");
		}
	}

	public boolean canSetStone (int x, int y, Stone stone)
	{
		int count = 0;

		//count += reverseUpper(x, y, stone, true);
		//count += reverseLower(x, y, stone, true);
		//count += reverseRight(x, y, stone, true);
		//count += reverseLeft(x, y, stone, true);

		//count += reverseUpperRight(x, y, stone, true);
		//count += reverseUpperLeft(x, y, stone, true);
		//count += reverseLowerRight(x, y, stone, true);
		//count += reverseLowerLeft(x, y, stone, true);

		System.out.println( count );

		if( countEmpty() >= 1 && count != 0 )
			return true;
		else
			return false;
	}

	public int countEmpty ()
	{
		int cnt = 0;
		
		for( int x=0; x < xLength_; x++ )
		{
			for( int y=0; y < yLength_; y++ )
			{
				if( field_[x][y].isEmpty() )
					cnt++;
			}
		}
		return cnt;
	}

	public void show ()
	{
		System.out.print("  ");
		for( int x=0; x < xLength_; x++ )
		{
			System.out.print(x + " ");
		}
		System.out.println();
	
		for( int y=0; y < yLength_; y++ )
		{
			System.out.print( y );

			for( int x=0; x < xLength_; x++ )
			{
				System.out.print("|");

				if( field_[x][y].isWhite() )
					System.out.print("o");

				else if( field_[x][y].isBlack() )
					System.out.print("*");

				else
					System.out.print(" ");
			}

			System.out.println("|");
		}
	}

	public Object clone()
	{
		try
		{
			Field newField = new Field( xLength_, yLength_ );
			for( int x = 0; x < xLength_; x++ )
			{
				for( int y = 0; y < yLength_; y++ )
				{
					newField.setStone( x, y, field_[x][y] );
				}
			}
			return newField;
		}
		catch ( OthelloException e )
		{
			e.printStackTrace();
		}
		return null;
	}
	
	// nan ko mitu katta ka kae su
	private int reverseUpper (int xPos, int yPos, Stone addStone, boolean exFlag)
	{
		int count = 0;
		boolean found = false;

		for( int y = yPos-1; y >= 0; y-- )
		{
			if( field_[xPos][y].isEmpty() )
				return 0;

			if( field_[xPos][y].equals( addStone ) )
				count++;
			else
				found = true;
		}

		if( found && count > 0)
		{
			for( int y = yPos-1; y >= 0; y-- )
			{
				if( field_[xPos][y].equals( addStone ) )
					break;

				if( !exFlag )
					field_[xPos][y].reverse();
			}
		}
		return count;
	}

	private int reverseLower (int xPos, int yPos, Stone stone, boolean exFlag)
	{
		boolean found = false;
		int count = 0;

		for( int y = yPos+1; y < yLength_; y++ )
		{
			if( field_[xPos][y].isEmpty() )
				break;

			if( field_[xPos][y].equals( stone ) )
				found = true;
			else
				count++;
		}
		if( !found  )
			return 0;

		for( int y = yPos+1; y >= 0; y-- )
		{
			if( exFlag )
				break;

			if( !field_[xPos][y].equals( stone ) )
				field_[xPos][y].reverse();
		}
		return count;
	}

	private int reverseLeft (int xPos, int yPos, Stone stone, boolean exFlag)
	{
		int foundCnt = 0;

		for( int x = xPos-1; x >= 0; x-- )
		{
			if( field_[x][yPos].isEmpty() )
				return 0;
		
			if( field_[x][yPos].equals( stone ) )
				foundCnt++;
		}

		if( foundCnt > 0)
		{
			for( int x = xPos-1; x >= 0; x-- )
			{
				if( field_[x][yPos].equals( stone ) )
					break;

				if( !exFlag )
					field_[x][yPos].reverse();
			}
		}
		return foundCnt;
	}
	
	private int reverseRight (int xPos, int yPos, Stone stone, boolean exFlag)
 	{
		int foundCnt = 0;

		for( int x = xPos+1; x < xLength_; x++ )
		{
			if( field_[x][yPos].isEmpty() )
				return 0;

			if( field_[x][yPos].equals( stone ) )
				foundCnt++;
		}

		if( foundCnt > 0 )
		{
			for( int x = xPos+1; x < xLength_; x++ )
			{
				if( field_[x][yPos].equals( stone ) )
					break;

				if( !exFlag )
					field_[x][yPos].reverse();
			}
		}
		return foundCnt;
	}

	public int reverseUpperRight (int xPos, int yPos, Stone stone, boolean exFlag)
	{
		int foundCnt = 0;

		for( int x = xPos+1; x < xLength_; x++ )
		{
			for( int y = yPos-1; y >= 0; y-- )
			{
				if( field_[x][y].equals( stone ) )
					return 0;

				if( field_[x][y].equals( stone ) )
					foundCnt++;
			}
		}
		if( foundCnt > 0 )
		{
			for( int x = xPos+1; x < xLength_; x++ )
			{
				for( int y = yPos-1; y >= 0; y-- )
				{
					if( field_[x][y].equals( stone ) )
						break;

					if( !exFlag )
						field_[x][y].reverse();
				}
			}
		}
		return foundCnt;
	}

	public int reverseUpperLeft (int xPos, int yPos, Stone stone, boolean exFlag)
	{
		int foundCnt = 0;

		for( int x = xPos-1; x >= 0; x-- )
		{
			for( int y = yPos-1; y >= 0; y-- )
			{
				if( field_[x][y].isEmpty() )
					return 0;

				if( field_[x][y].equals( stone ))
					foundCnt++;
			}
		}

		if( foundCnt > 0 )
		{
			for( int x = xPos-1; x >= 0; x-- )
			{
				for( int y = yPos-1; y >= 0; y-- )
				{
					if( field_[x][y].equals( stone ) )
						break;

					if( !exFlag )
						field_[x][y].reverse();
				}
			}
		}
		return foundCnt;
	}

	public int reverseLowerRight (int xPos, int yPos, Stone stone, boolean exFlag)
	{
		int foundCnt = 0;

		for( int x = xPos+1; x < xLength_; x++ )
		{
			for( int y = yPos+1; y < yLength_; y++ )
			{
				if( field_[x][y].isEmpty() )
					return 0;

				if( field_[x][y].equals( stone ))
					foundCnt++;
			}
		}

		if( foundCnt > 0 )
		{
			for( int x = xPos+1; x < xLength_; x++ )
			{
				for( int y = yPos+1; y < yLength_; y++ )
				{
					if( field_[x][y].equals( stone ) )
						break;

					if( !exFlag )
						field_[x][y].reverse();
				}
			}
		}
		return foundCnt;
	}
	public int reverseLowerLeft (int xPos, int yPos, Stone stone, boolean exFlag)
	{
		int foundCnt = 0;

		for( int x = xPos-1; x >= 0; x-- )
		{
			for( int y = yPos+1; y < yLength_; y++ )
			{
				if( field_[x][y].isEmpty() )
					return 0;

				if( field_[x][y].equals( stone ) )
					foundCnt++;

			}
		}

		if( foundCnt > 0 )
		{
			for( int x = xPos-1; x >= 0; x-- )
			{
				for( int y = yPos+1; y < yLength_; y++ )
				{
					if( field_[x][y].equals( stone ) )
						break;

					if( !exFlag )
						field_[x][y].reverse();
				}
			}
		}
		return foundCnt;
	}
}

// プレゼンテーション
public class Othello
{
	private static String getLine ()
	{
		try
		{
			BufferedReader reader = 
				new BufferedReader(
						new InputStreamReader(
							System.in
							)
						);

			return reader.readLine();
		}
		catch( IOException e )
		{
			return null;
		}
	}

	public static int stringToInt (String str)
	{
		try
		{
			Integer intObj = Integer.valueOf( str );
			return intObj.intValue();
		}
		catch( NumberFormatException e )
		{
			return -1;
		}
	}

	public static void main(String[] ap)
		throws OthelloException
	{
		// field create!!
		Field field = new Field(5, 5);

		// default is player1
		boolean player1Turn = true;
		
		// create stone obj
		Stone blackStone = new Stone( Stone.BLACK );
		Stone whiteStone = new Stone( Stone.WHITE );

		while( true )
		{
			field.show();

			// wait player's input
			System.out.print("please input: ");
			String line = getLine();
			
			// exit check
			if( line == null || line.equals("quit") )
			{
				System.exit(0);
			}

			// parse user input string
			StringTokenizer tkn =
				new StringTokenizer(line, ", ");
			if( tkn.countTokens() != 2 )
			{
				System.out.println("token error!!");
				continue;
			}
			
			// convert to integer value
			int x = stringToInt( tkn.nextToken() );
			int y = stringToInt( tkn.nextToken() );
			if( x == -1 || y == -1)
			{
				System.out.println("not invalid value!!");
				continue;
			}

			// player1 or player2
			Stone putStone = player1Turn ? blackStone : whiteStone;
			try
			{
				field.setStone(x, y, putStone);
			}
			catch ( OthelloException e )
			{
				System.out.println("error retry!!");
				continue;
			}

			// change player. next turn!!
			player1Turn = !player1Turn;
		}

		// show result
	}
}
