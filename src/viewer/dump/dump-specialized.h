/* Fully specialized functions for dumping to a file.
 *
 * This include file defines a function that dumps the document to a
 * file.  The function is specialized to one color mode and one kind
 * of charset.  This is supposedly faster than runtime checks.  The
 * file that includes this file must define several macros to select
 * the specialization.
 *
 * The following macro must be defined:
 *
 * - DUMP_FUNCTION_SPECIALIZED: The name of the function that this
 *   file should define.
 *
 * At most one of the following macros may be defined:
 *
 * - DUMP_COLOR_MODE_16
 * - DUMP_COLOR_MODE_256
 * - DUMP_COLOR_MODE_TRUE
 *
 * The following macro may be defined:
 *
 * - DUMP_CHARSET_UTF8
 */

static int
DUMP_FUNCTION_SPECIALIZED(struct document *document, int fd,
			  unsigned char buf[D_BUF])
{
	int y;
	int bptr = 0;
#ifdef DUMP_COLOR_MODE_16
	unsigned char color = 0;
	int width = get_opt_int("document.dump.width");
#elif defined(DUMP_COLOR_MODE_256)
	unsigned char foreground = 0;
	unsigned char background = 0;
	int width = get_opt_int("document.dump.width");
#elif defined(DUMP_COLOR_MODE_TRUE)
	static unsigned char color[6] = {255, 255, 255, 0, 0, 0};
	unsigned char *foreground = &color[0];
	unsigned char *background = &color[3];
	int width = get_opt_int("document.dump.width");
#endif	/* DUMP_COLOR_MODE_TRUE */

	for (y = 0; y < document->height; y++) {
		int white = 0;
		int x;

#ifdef DUMP_COLOR_MODE_16
		write_color_16(color, fd, buf, &bptr);
#elif defined(DUMP_COLOR_MODE_256)
		write_color_256("38", foreground, fd, buf, &bptr);
		write_color_256("48", background, fd, buf, &bptr);
#elif defined(DUMP_COLOR_MODE_TRUE)
		write_true_color("38", foreground, fd, buf, &bptr);
		write_true_color("48", background, fd, buf, &bptr);
#endif	/* DUMP_COLOR_MODE_TRUE */

		for (x = 0; x < document->data[y].length; x++) {
#ifdef DUMP_CHARSET_UTF8
			unicode_val_T c;
#else
			unsigned char c;
#endif
			unsigned char attr = document->data[y].chars[x].attr;
#ifdef DUMP_COLOR_MODE_16
			unsigned char color1 = document->data[y].chars[x].color[0];

			if (color != color1) {
				color = color1;
				if (write_color_16(color, fd, buf, &bptr))
					return -1;
			}
#elif defined(DUMP_COLOR_MODE_256)
			unsigned char color1 = document->data[y].chars[x].color[0];
			unsigned char color2 = document->data[y].chars[x].color[1];

			if (foreground != color1) {
				foreground = color1;
				if (write_color_256("38", foreground, fd, buf, &bptr))
					return -1;
			}

			if (background != color2) {
				background = color2;
				if (write_color_256("48", background, fd, buf, &bptr))
					return -1;
			}
#elif defined(DUMP_COLOR_MODE_TRUE)
			unsigned char *new_foreground = &document->data[y].chars[x].color[0];
			unsigned char *new_background = &document->data[y].chars[x].color[3];

			if (memcmp(foreground, new_foreground, 3)) {
				foreground = new_foreground;
				if (write_true_color("38", foreground, fd, buf, &bptr))
					return -1;
			}

			if (memcmp(background, new_background, 3)) {
				background = new_background;
				if (write_true_color("48", background, fd, buf, &bptr))
					return -1;
			}
#endif	/* DUMP_COLOR_MODE_TRUE */

			c = document->data[y].chars[x].data;

			if ((attr & SCREEN_ATTR_FRAME)
			    && c >= 176 && c < 224)
				c = frame_dumb[c - 176];

			if (c <= ' ') {
				/* Count spaces. */
				white++;
				continue;
			}

			/* Print spaces if any. */
			while (white) {
				if (write_char(' ', fd, buf, &bptr))
					return -1;
				white--;
			}

			/* Print normal char. */
#ifdef DUMP_CHARSET_UTF8
			{
				unsigned char *utf8_buf = encode_utf8(c);

				while (*utf8_buf) {
					if (write_char(*utf8_buf++,
						       fd, buf, &bptr)) return -1;
				}

				x += unicode_to_cell(c) - 1;
			}
#else  /* !DUMP_CHARSET_UTF8 */
			if (write_char(c, fd, buf, &bptr))
				return -1;
#endif	/* !DUMP_CHARSET_UTF8 */
		}

#if defined(DUMP_COLOR_MODE_16) || defined(DUMP_COLOR_MODE_256) || defined(DUMP_COLOR_MODE_TRUE)
		for (;x < width; x++) {
			if (write_char(' ', fd, buf, &bptr))
				return -1;
		}
#endif	/* DUMP_COLOR_MODE_16 || DUMP_COLOR_MODE_256 || DUMP_COLOR_MODE_TRUE */

		/* Print end of line. */
		if (write_char('\n', fd, buf, &bptr))
			return -1;
	}

	if (hard_write(fd, buf, bptr) != bptr)
		return -1;

	return 0;
}
