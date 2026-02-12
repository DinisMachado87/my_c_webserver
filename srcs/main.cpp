#include "Token.hpp"
#include "server/Server.hpp"
#include <string>
int	main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	
	Token token(Token::configDelimiters());
	std::string str("Hello earth;\nHere I am;");
	unsigned char *cur = (unsigned char*)str.c_str();
	while(1) {
		cur = token.next(cur);
		if (!cur)
			break;
		token.printToken();
	}
	return (0);
}
