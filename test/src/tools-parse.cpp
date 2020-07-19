#include "catch2/catch.hpp"

#ifndef __LIBARCSTK_IDENTIFIER_HPP__
#include <arcstk/identifier.hpp>
#endif

#ifndef __ARCSTOOLS_TOOLS_PARSE_HPP__
#include "tools-parse.hpp"
#endif


TEST_CASE ( "ARFileParser", "[parse] [arfileparser]" )
{
	using ARFileParser = arcsapp::ARFileParser;

	using ARId                  = arcstk::ARId;
	using ARResponse            = arcstk::ARResponse;
	using DefaultErrorHandler   = arcstk::DefaultErrorHandler;
	using DefaultContentHandler = arcstk::DefaultContentHandler;

	ARFileParser parser;
	ARResponse result;

	auto c_handler = std::make_unique<DefaultContentHandler>();
	c_handler->set_object(result);

	// content handler but no error handler
	parser.set_content_handler(std::move(c_handler));

	SECTION ( "Not copyable" )
	{
		CHECK ( not std::is_copy_constructible<ARFileParser>::value );
		CHECK ( not std::is_copy_assignable<ARFileParser>::value    );
	}

	SECTION ( "Move constructor" )
	{
		auto e_handler = std::make_unique<DefaultErrorHandler>();
		parser.set_error_handler(std::move(e_handler));
		parser.set_file("foo");

		auto c_handler_adr = &parser.content_handler();
		auto e_handler_adr = &parser.error_handler();

		ARFileParser fileparser_moved(std::move(parser));

		CHECK ( &fileparser_moved.content_handler() == c_handler_adr );
		CHECK ( &fileparser_moved.error_handler()   == e_handler_adr );
		CHECK ( fileparser_moved.file()             == "foo" );
	}

	SECTION ( "Move assignment" )
	{
		auto e_handler = std::make_unique<DefaultErrorHandler>();
		parser.set_error_handler(std::move(e_handler));
		parser.set_file("foo");

		auto c_handler_adr = &parser.content_handler();
		auto e_handler_adr = &parser.error_handler();

		ARFileParser fileparser_moved = std::move(parser);

		CHECK ( &fileparser_moved.content_handler() == c_handler_adr );
		CHECK ( &fileparser_moved.error_handler()   == e_handler_adr );
		CHECK ( fileparser_moved.file()             == "foo" );
	}

	SECTION ( "Parse valid file correctly" )
	{
		parser.set_file("dBAR-015-001b9178-014be24e-b40d2d0f.bin");
		parser.parse();


		CHECK ( result.size() == 3 );

		// Block 1

		CHECK ( result[0].id() ==
				ARId(15, 0x001b9178, 0x014be24e, 0xb40d2d0f) );

		CHECK ( result[0].size() == 15 );

		CHECK ( result[0][0].arcs()           == 0xB89992E5 );
		CHECK ( result[0][0].confidence()     == 24 );
		CHECK ( result[0][0].frame450_arcs()  == 0x126D875E );

		CHECK ( result[0][1].arcs()           == 0x4F77EB03  );
		CHECK ( result[0][1].confidence()     == 24 );
		CHECK ( result[0][1].frame450_arcs()  == 0xABF770DA );

		CHECK ( result[0][2].arcs()           == 0x56582282 );
		CHECK ( result[0][2].confidence()     == 24 );
		CHECK ( result[0][2].frame450_arcs()  == 0x80ACAF3C );

		CHECK ( result[0][3].arcs()           == 0x9E2187F9  );
		CHECK ( result[0][3].confidence()     == 24 );
		CHECK ( result[0][3].frame450_arcs()  == 0x8EB77C86 );

		CHECK ( result[0][4].arcs()           == 0x6BE71E50  );
		CHECK ( result[0][4].confidence()     == 24 );
		CHECK ( result[0][4].frame450_arcs()  == 0xF62D90FC );

		CHECK ( result[0][5].arcs()           == 0x01E7235F  );
		CHECK ( result[0][5].confidence()     == 24 );
		CHECK ( result[0][5].frame450_arcs()  == 0x56C6AF06 );

		CHECK ( result[0][6].arcs()           == 0xD8F7763C );
		CHECK ( result[0][6].confidence()     == 24 ) ;
		CHECK ( result[0][6].frame450_arcs()  == 0x76274140 );

		CHECK ( result[0][7].arcs()           == 0x8480223E );
		CHECK ( result[0][7].confidence()     == 24 ) ;
		CHECK ( result[0][7].frame450_arcs()  == 0x73A608D0 );

		CHECK ( result[0][8].arcs()           == 0x42C5061C );
		CHECK ( result[0][8].confidence()     == 24 ) ;
		CHECK ( result[0][8].frame450_arcs()  == 0x9D7A1F4B );

		CHECK ( result[0][9].arcs()           == 0x47A70F02 );
		CHECK ( result[0][9].confidence()     == 23 ) ;
		CHECK ( result[0][9].frame450_arcs()  == 0x37871A8C );

		CHECK ( result[0][10].arcs()          == 0xBABF08CC );
		CHECK ( result[0][10].confidence()    == 23 ) ;
		CHECK ( result[0][10].frame450_arcs() == 0xF6360C0B );

		CHECK ( result[0][11].arcs()          == 0x563EDCCB );
		CHECK ( result[0][11].confidence()    == 23 ) ;
		CHECK ( result[0][11].frame450_arcs() == 0xCB1FE45D );

		CHECK ( result[0][12].arcs()          == 0xAB123C7C );
		CHECK ( result[0][12].confidence()    == 23 ) ;
		CHECK ( result[0][12].frame450_arcs() == 0xBCC08EDA );

		CHECK ( result[0][13].arcs()          == 0xC65C20E4 );
		CHECK ( result[0][13].confidence()    == 22 ) ;
		CHECK ( result[0][13].frame450_arcs() == 0xE467DE8E );

		CHECK ( result[0][14].arcs()          == 0x58FC3C3E );
		CHECK ( result[0][14].confidence()    == 22 ) ;
		CHECK ( result[0][14].frame450_arcs() == 0x9537953F );

		// Block 2

		CHECK ( result[1].id() ==
				ARId(15, 0x001b9178, 0x014be24e, 0xb40d2d0f) );

		CHECK ( result[1].size() == 15 );

		CHECK ( result[1][0].arcs()           == 0x98B10E0F );
		CHECK ( result[1][0].confidence()     == 20 );
		CHECK ( result[1][0].frame450_arcs()  == 0x35DC25F3 );

		CHECK ( result[1][1].arcs()           == 0x475F57E9 );
		CHECK ( result[1][1].confidence()     == 20 );
		CHECK ( result[1][1].frame450_arcs()  == 0xDCCF2356 );

		CHECK ( result[1][2].arcs()           == 0x7304F1C4 );
		CHECK ( result[1][2].confidence()     == 20 );
		CHECK ( result[1][2].frame450_arcs()  == 0x5FA89D66 );

		CHECK ( result[1][3].arcs()           == 0xF2472287 );
		CHECK ( result[1][3].confidence()     == 20 );
		CHECK ( result[1][3].frame450_arcs()  == 0xB0330387 );

		CHECK ( result[1][4].arcs()           == 0x881BC504 );
		CHECK ( result[1][4].confidence()     == 20 );
		CHECK ( result[1][4].frame450_arcs()  == 0x8442806E );

		CHECK ( result[1][5].arcs()           == 0xBB94BFD4 );
		CHECK ( result[1][5].confidence()     == 20 );
		CHECK ( result[1][5].frame450_arcs()  == 0xF13BC09B );

		CHECK ( result[1][6].arcs()           == 0xF9CAEE76 );
		CHECK ( result[1][6].confidence()     == 20 );
		CHECK ( result[1][6].frame450_arcs()  == 0xC0AB9412 );

		CHECK ( result[1][7].arcs()           == 0xF9F60BC1 );
		CHECK ( result[1][7].confidence()     == 20 );
		CHECK ( result[1][7].frame450_arcs()  == 0xC7836441 );

		CHECK ( result[1][8].arcs()           == 0x2C736302 );
		CHECK ( result[1][8].confidence()     == 19 );
		CHECK ( result[1][8].frame450_arcs()  == 0xF1FD38D3 );

		CHECK ( result[1][9].arcs()           == 0x1C955978 );
		CHECK ( result[1][9].confidence()     == 20 );
		CHECK ( result[1][9].frame450_arcs()  == 0x0860E08B );

		CHECK ( result[1][10].arcs()          == 0xFDA6D833 );
		CHECK ( result[1][10].confidence()    == 19 );
		CHECK ( result[1][10].frame450_arcs() == 0x9C8202BE );

		CHECK ( result[1][11].arcs()          == 0x3A57E5D1 );
		CHECK ( result[1][11].confidence()    == 21 );
		CHECK ( result[1][11].frame450_arcs() == 0xC6FF01AE );

		CHECK ( result[1][12].arcs()          == 0x6ED5F3E7 );
		CHECK ( result[1][12].confidence()    == 19 );
		CHECK ( result[1][12].frame450_arcs() == 0x4C92FCBE );

		CHECK ( result[1][13].arcs()          == 0x4A5C3872 );
		CHECK ( result[1][13].confidence()    == 20 );
		CHECK ( result[1][13].frame450_arcs() == 0x8FB684A7 );

		CHECK ( result[1][14].arcs()          == 0x5FE8B032 );
		CHECK ( result[1][14].confidence()    == 19 );
		CHECK ( result[1][14].frame450_arcs() == 0x405711AA );

		// Block 3

		CHECK ( result[2][0].arcs()           == 0x54FE0533 );
		CHECK ( result[2][0].confidence()     == 2 );
		CHECK ( result[2][0].frame450_arcs()  == 0 );

		CHECK ( result[2][1].arcs()           == 0xAFEC147E );
		CHECK ( result[2][1].confidence()     == 2 );
		CHECK ( result[2][1].frame450_arcs()  == 0 );

		CHECK ( result[2][2].arcs()           == 0x2BFB5AEC );
		CHECK ( result[2][2].confidence()     == 2 );
		CHECK ( result[2][2].frame450_arcs()  == 0 );

		CHECK ( result[2][3].arcs()           == 0xED6E7215 );
		CHECK ( result[2][3].confidence()     == 2 );
		CHECK ( result[2][3].frame450_arcs()  == 0 );

		CHECK ( result[2][4].arcs()           == 0xA0847CEF );
		CHECK ( result[2][4].confidence()     == 2 );
		CHECK ( result[2][4].frame450_arcs()  == 0 );

		CHECK ( result[2][5].arcs()           == 0xAE066CD1 );
		CHECK ( result[2][5].confidence()     == 2 );
		CHECK ( result[2][5].frame450_arcs()  == 0 );

		CHECK ( result[2][6].arcs()           == 0xE1AB3B46 );
		CHECK ( result[2][6].confidence()     == 2 );
		CHECK ( result[2][6].frame450_arcs()  == 0 );

		CHECK ( result[2][7].arcs()           == 0xE75E70BA );
		CHECK ( result[2][7].confidence()     == 2 );
		CHECK ( result[2][7].frame450_arcs()  == 0 );

		CHECK ( result[2][8].arcs()           == 0x6D72D1AC );
		CHECK ( result[2][8].confidence()     == 2 );
		CHECK ( result[2][8].frame450_arcs()  == 0 );

		CHECK ( result[2][9].arcs()           == 0x89C19A02 );
		CHECK ( result[2][9].confidence()     == 2 );
		CHECK ( result[2][9].frame450_arcs()  == 0 );

		CHECK ( result[2][10].arcs()          == 0x4A5CE2AB );
		CHECK ( result[2][10].confidence()    == 2 );
		CHECK ( result[2][10].frame450_arcs() == 0 );

		CHECK ( result[2][11].arcs()          == 0x4D23C1D4 );
		CHECK ( result[2][11].confidence()    == 2 );
		CHECK ( result[2][11].frame450_arcs() == 0 );

		CHECK ( result[2][12].arcs()          == 0x80AA0FB6 );
		CHECK ( result[2][12].confidence()    == 2 );
		CHECK ( result[2][12].frame450_arcs() == 0 );

		CHECK ( result[2][13].arcs()          == 0x9378FD52 );
		CHECK ( result[2][13].confidence()    == 2 );
		CHECK ( result[2][13].frame450_arcs() == 0 );

		CHECK ( result[2][14].arcs()          == 0x6A8A614C );
		CHECK ( result[2][14].confidence()    == 2 );
		CHECK ( result[2][14].frame450_arcs() == 0 );
	}

	// A parse error can only occurr if the input ends prematurely, e.g. if
	// an input block, for any reason, is not 13 + (n * 9) bytes long (with n
	// being the track number.
	//
	// 22 cases are tested:
	//
	// Header is 13 bytes long, hence there are 13 positions (after byte 1 - 13)
	// for errors that are covered by the input files *H_01-H_013.
	//
	// Triplet is 9 bytes long, hence there are 9 different positions (before
	// byte 1 and after byte 1 - 8) for errors that are covered by the input
	// files *T_00-T_08.

	// There is no easy or comfortable way in Catch2 to access the
	// exception object thrown via its genuine interface. (However, you can
	// access the "what" message via CHECK_THROWS_WITH).
	//
	// The following workaround addresses this by requiring an exception
	// manually and catching the exception object.
	//
	// Confer: https://github.com/catchorg/Catch2/issues/394


	SECTION ( "Parse files with incomplete header: no disc id1 (pos 1)" )
	{
		parser.set_file("dBAR-015-001b9178-014be24e-b40d2d0f_H+01.bin");
		try
		{
			parser.parse();

			FAIL ( "Expected StreamReadException was not thrown" );
		} catch (const arcstk::StreamReadException &e)
		{
			CHECK ( e.block()               == 2 );
			CHECK ( e.block_byte_position() == 1 );
			CHECK ( e.byte_position()       == 149 );
		}
	}


	SECTION ( "Parse files with incomplete header: disc id1 (pos 2, 3 or 4)" )
	{
		parser.set_file("dBAR-015-001b9178-014be24e-b40d2d0f_H+02.bin");
		try
		{
			parser.parse();

			FAIL ( "Expected StreamReadException was not thrown" );
		} catch (const arcstk::StreamReadException &e)
		{
			CHECK ( e.block()               == 2 );
			CHECK ( e.block_byte_position() == 2 );
			CHECK ( e.byte_position()       == 150 );
		}

		parser.set_file("dBAR-015-001b9178-014be24e-b40d2d0f_H+03.bin");
		try
		{
			parser.parse();

			FAIL ( "Expected StreamReadException was not thrown" );
		} catch (const arcstk::StreamReadException &e)
		{
			CHECK ( e.block()               == 2 );
			CHECK ( e.block_byte_position() == 3 );
			CHECK ( e.byte_position()       == 151 );
		}

		parser.set_file("dBAR-015-001b9178-014be24e-b40d2d0f_H+04.bin");
		try
		{
			parser.parse();

			FAIL ( "Expected StreamReadException was not thrown" );
		} catch (const arcstk::StreamReadException &e)
		{
			CHECK ( e.block()               == 2 );
			CHECK ( e.block_byte_position() == 4 );
			CHECK ( e.byte_position()       == 152 );
		}
	}


	SECTION ( "Parse files with incomplete header: no disc id2 (pos 5)" )
	{
		parser.set_file("dBAR-015-001b9178-014be24e-b40d2d0f_H+05.bin");
		try
		{
			parser.parse();

			FAIL ( "Expected StreamReadException was not thrown" );
		} catch (const arcstk::StreamReadException &e)
		{
			CHECK ( e.block()               == 2 );
			CHECK ( e.block_byte_position() == 5 );
			CHECK ( e.byte_position()       == 153 );
		}
	}


	SECTION ( "Parse files with incomplete header: disc id2 (pos 6, 7 or 8)" )
	{
		parser.set_file("dBAR-015-001b9178-014be24e-b40d2d0f_H+06.bin");
		try
		{
			parser.parse();

			FAIL ( "Expected StreamReadException was not thrown" );
		} catch (const arcstk::StreamReadException &e)
		{
			CHECK ( e.block()               == 2 );
			CHECK ( e.block_byte_position() == 6 );
			CHECK ( e.byte_position()       == 154 );
		}

		parser.set_file("dBAR-015-001b9178-014be24e-b40d2d0f_H+07.bin");
		try
		{
			parser.parse();

			FAIL ( "Expected StreamReadException was not thrown" );
		} catch (const arcstk::StreamReadException &e)
		{
			CHECK ( e.block()               == 2 );
			CHECK ( e.block_byte_position() == 7 );
			CHECK ( e.byte_position()       == 155 );
		}

		parser.set_file("dBAR-015-001b9178-014be24e-b40d2d0f_H+08.bin");
		try
		{
			parser.parse();

			FAIL ( "Expected StreamReadException was not thrown" );
		} catch (const arcstk::StreamReadException &e)
		{
			CHECK ( e.block()               == 2 );
			CHECK ( e.block_byte_position() == 8 );
			CHECK ( e.byte_position()       == 156 );
		}
	}


	SECTION ( "Parse files with incomplete header: no cddb id (pos 9)" )
	{
		parser.set_file("dBAR-015-001b9178-014be24e-b40d2d0f_H+09.bin");
		try
		{
			parser.parse();

			FAIL ( "Expected StreamReadException was not thrown" );
		} catch (const arcstk::StreamReadException &e)
		{
			CHECK ( e.block()               == 2 );
			CHECK ( e.block_byte_position() == 9 );
			CHECK ( e.byte_position()       == 157 );
		}
	}


	SECTION ( "Parse files with incomplete header: cddb id (pos 10, 11 or 12)" )
	{
		parser.set_file("dBAR-015-001b9178-014be24e-b40d2d0f_H+10.bin");
		try
		{
			parser.parse();

			FAIL ( "Expected StreamReadException was not thrown" );
		} catch (const arcstk::StreamReadException &e)
		{
			CHECK ( e.block()               == 2 );
			CHECK ( e.block_byte_position() == 10 );
			CHECK ( e.byte_position()       == 158 );
		}

		parser.set_file("dBAR-015-001b9178-014be24e-b40d2d0f_H+11.bin");
		try
		{
			parser.parse();

			FAIL ( "Expected StreamReadException was not thrown" );
		} catch (const arcstk::StreamReadException &e)
		{
			CHECK ( e.block()               == 2 );
			CHECK ( e.block_byte_position() == 11 );
			CHECK ( e.byte_position()       == 159 );
		}

		parser.set_file("dBAR-015-001b9178-014be24e-b40d2d0f_H+12.bin");
		try
		{
			parser.parse();

			FAIL ( "Expected StreamReadException was not thrown" );
		} catch (const arcstk::StreamReadException &e)
		{
			CHECK ( e.block()               == 2 );
			CHECK ( e.block_byte_position() == 12 );
			CHECK ( e.byte_position()       == 160 );
		}
	}


	SECTION ( "Parse files with incomplete block: only header" )
	{
		parser.set_file("dBAR-015-001b9178-014be24e-b40d2d0f_H+13.bin");
		try
		{
			parser.parse();

			FAIL ( "Expected StreamReadException was not thrown" );
		} catch (const arcstk::StreamReadException &e)
		{
			CHECK ( e.block()               == 2 );
			CHECK ( e.block_byte_position() == 13 );
			CHECK ( e.byte_position()       == 161 );
		}
	}


	SECTION ( "Parse files with triplet missing (triplet pos 0)" )
	{
		parser.set_file("dBAR-015-001b9178-014be24e-b40d2d0f_T+0.bin");
		try
		{
			// End of a triplet + 0 byte => one or more triplets missing

			parser.parse();

			FAIL ( "Expected StreamReadException was not thrown" );
		} catch (const arcstk::StreamReadException &e)
		{
			CHECK ( e.block()               == 2 );
			CHECK ( e.block_byte_position() == 139 );
			CHECK ( e.byte_position()       == 287 );
		}
	}


	SECTION ( "Parse files with missing ARCS (triplet pos 1)" )
	{
		parser.set_file("dBAR-015-001b9178-014be24e-b40d2d0f_T+1.bin");
		try
		{
			// End of last triplet + 1 byte =>
			// triplet invalid, confidence ok, ARCS missing

			parser.parse();

			FAIL ( "Expected StreamReadException was not thrown" );
		} catch (const arcstk::StreamReadException &e)
		{
			CHECK ( e.block()               == 2 );
			CHECK ( e.block_byte_position() == 140 );
			CHECK ( e.byte_position()       == 288 );
		}
	}


	SECTION ( "Parse files with broken ARCS (triplet pos 2,3 or 4)" )
	{
		// End of last triplet + 2,3 or 4 bytes =>
		// triplet invalid, confidence ok, ARCS incomplete

		parser.set_file("dBAR-015-001b9178-014be24e-b40d2d0f_T+2.bin");
		try
		{
			parser.parse();

			FAIL ( "Expected StreamReadException was not thrown" );
		} catch (const arcstk::StreamReadException &e)
		{
			CHECK ( e.block()               == 2 );
			CHECK ( e.block_byte_position() == 141 );
			CHECK ( e.byte_position()       == 289 );
		}

		parser.set_file("dBAR-015-001b9178-014be24e-b40d2d0f_T+3.bin");
		try
		{
			parser.parse();

			FAIL ( "Expected StreamReadException was not thrown" );
		} catch (const arcstk::StreamReadException &e)
		{
			CHECK ( e.block()               == 2 );
			CHECK ( e.block_byte_position() == 142 );
			CHECK ( e.byte_position()       == 290 );
		}

		parser.set_file("dBAR-015-001b9178-014be24e-b40d2d0f_T+4.bin");
		try
		{
			parser.parse();

			FAIL ( "Expected StreamReadException was not thrown" );
		} catch (const arcstk::StreamReadException &e)
		{
			CHECK ( e.block()               == 2 );
			CHECK ( e.block_byte_position() == 143 );
			CHECK ( e.byte_position()       == 291 );
		}
	}


	SECTION ( "Parse files with missing frame450_arcs (triplet pos 5)" )
	{
		// End of last triplet + 5 bytes =>
		// triplet invalid, confidence + ARCS ok, frame450_arcs missing

		parser.set_file("dBAR-015-001b9178-014be24e-b40d2d0f_T+5.bin");
		try
		{
			parser.parse();

			FAIL ( "Expected StreamReadException was not thrown" );
		} catch (const arcstk::StreamReadException &e)
		{
			CHECK ( e.block()               == 2 );
			CHECK ( e.block_byte_position() == 144 );
			CHECK ( e.byte_position()       == 292 );
		}
	}


	SECTION ( "Parse files with broken frame450_arcs (triplet pos 6,7 or 8)" )
	{
		// End of last triplet + 6,7 or 8 bytes =>
		// triplet invalid, confidence + ARCS ok, frame450_arcs incomplete

		parser.set_file("dBAR-015-001b9178-014be24e-b40d2d0f_T+6.bin");
		try
		{
			parser.parse();

			FAIL ( "Expected StreamReadException was not thrown" );
		} catch (const arcstk::StreamReadException &e)
		{
			CHECK ( e.block()               == 2 );
			CHECK ( e.block_byte_position() == 145 );
			CHECK ( e.byte_position()       == 293 );
		}

		parser.set_file("dBAR-015-001b9178-014be24e-b40d2d0f_T+7.bin");
		try
		{
			parser.parse();

			FAIL ( "Expected StreamReadException was not thrown" );
		} catch (const arcstk::StreamReadException &e)
		{
			CHECK ( e.block()               == 2 );
			CHECK ( e.block_byte_position() == 146 );
			CHECK ( e.byte_position()       == 294 );
		}

		parser.set_file("dBAR-015-001b9178-014be24e-b40d2d0f_T+8.bin");
		try
		{
			parser.parse();

			FAIL ( "Expected StreamReadException was not thrown" );
		} catch (const arcstk::StreamReadException &e)
		{
			CHECK ( e.block()               == 2 );
			CHECK ( e.block_byte_position() == 147 );
			CHECK ( e.byte_position()       == 295 );
		}
	}
}


TEST_CASE ( "ARStdinParser", "[parse] [arstdinparser]" )
{
	using ARStdinParser = arcstk::ARStdinParser;
	using ARResponse   = arcstk::ARResponse;
	using DefaultErrorHandler   = arcstk::DefaultErrorHandler;
	using DefaultContentHandler = arcstk::DefaultContentHandler;

	ARStdinParser parser;
	ARResponse result;

	auto c_handler = std::make_unique<DefaultContentHandler>();
	c_handler->set_object(result);

	// content handler but no error handler
	parser.set_content_handler(std::move(c_handler));

	SECTION ( "Not copyable" )
	{
		CHECK ( not std::is_copy_constructible<ARStdinParser>::value );
		CHECK ( not std::is_copy_assignable<ARStdinParser>::value    );
	}

	SECTION ( "Move constructor" )
	{
		auto e_handler = std::make_unique<DefaultErrorHandler>();
		parser.set_error_handler(std::move(e_handler));

		auto c_handler_adr = &parser.content_handler();
		auto e_handler_adr = &parser.error_handler();

		ARStdinParser stdinparser_moved(std::move(parser));

		CHECK ( &stdinparser_moved.content_handler() == c_handler_adr );
		CHECK ( &stdinparser_moved.error_handler()   == e_handler_adr );
	}

	SECTION ( "Move assignment" )
	{
		auto e_handler = std::make_unique<DefaultErrorHandler>();
		parser.set_error_handler(std::move(e_handler));

		auto c_handler_adr = &parser.content_handler();
		auto e_handler_adr = &parser.error_handler();

		ARStdinParser stdinparser_moved = std::move(parser);

		CHECK ( &stdinparser_moved.content_handler() == c_handler_adr );
		CHECK ( &stdinparser_moved.error_handler()   == e_handler_adr );
	}
}
