/* character-name table */
static struct cname {
	char *name;
	char code;
} cnames[] = {
	"NUL",	'\0',
	"SOH",	'\001',
	"STX",	'\002',
	"ETX",	'\003',
	"EOT",	'\004',
	"ENQ",	'\005',
	"ACK",	'\006',
	"BEL",	'\007',
	"alert",	'\007',
	"BS",		'\010',
	"backspace",	'\b',
	"HT",		'\011',
	"tab",		'\t',
	"LF",		'\012',
	"newline",	'\n',
	"VT",		'\013',
	"vertical-tab",	'\v',
	"FF",		'\014',
	"form-feed",	'\f',
	"CR",		'\015',
	"carriage-return",	'\r',
	"SO",	'\016',
	"SI",	'\017',
	"DLE",	'\020',
	"DC1",	'\021',
	"DC2",	'\022',
	"DC3",	'\023',
	"DC4",	'\024',
	"NAK",	'\025',
	"SYN",	'\026',
	"ETB",	'\027',
	"CAN",	'\030',
	"EM",	'\031',
	"SUB",	'\032',
	"ESC",	'\033',
	"IS4",	'\034',
	"FS",	'\034',
	"IS3",	'\035',
	"GS",	'\035',
	"IS2",	'\036',
	"RS",	'\036',
	"IS1",	'\037',
	"US",	'\037',
	"space",		' ',
	"exclamation-mark",	'!',
	"quotation-mark",	'"',
	"number-sign",		'#',
	"dollar-sign",		'$',
	"percent-sign",		'%',
	"ampersand",		'&',
	"apostrophe",		'\'',
	"left-parenthesis",	'(',
	"right-parenthesis",	')',
	"asterisk",	'*',
	"plus-sign",	'+',
	"comma",	',',
	"hyphen",	'-',
	"hyphen-minus",	'-',
	"period",	'.',
	"full-stop",	'.',
	"slash",	'/',
	"solidus",	'/',
	"zero",		'0',
	"one",		'1',
	"two",		'2',
	"three",	'3',
	"four",		'4',
	"five",		'5',
	"six",		'6',
	"seven",	'7',
	"eight",	'8',
	"nine",		'9',
	"colon",	':',
	"semicolon",	';',
	"less-than-sign",	'<',
	"equals-sign",		'=',
	"greater-than-sign",	'>',
	"question-mark",	'?',
	"commercial-at",	'@',
	"left-square-bracket",	'[',
	"backslash",		'\\',
	"reverse-solidus",	'\\',
	"right-square-bracket",	']',
	"circumflex",		'^',
	"circumflex-accent",	'^',
	"underscore",		'_',
	"low-line",		'_',
	"grave-accent",		'`',
	"left-brace",		'{',
	"left-curly-bracket",	'{',
	"vertical-line",	'|',
	"right-brace",		'}',
	"right-curly-bracket",	'}',
	"tilde",		'~',
	"DEL",	'\177',
	NULL,	0,
};
