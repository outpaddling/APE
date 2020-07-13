/* ape.c */
int main(int argc, char *argv[]);
void nice_exit(int retval, err_t *errfile, char *message);
int eat_keys(unsigned long time, char seq[]);
/* build_m.c */
int build_menu(file_t files[], int *af_ptr, proj_t *project, opt_t *options, err_t *errfile, buff_t *cut_buff, event_t *event);
void create_build_menu(char *build_text[], proj_t *project, lang_t *lang);
void debug(file_t *file, proj_t *project, char *trace_cmd, opt_t *options);
int compile_prog(file_t files[], int af, err_t *errfile, opt_t *options, out_t output);
int run_prog(file_t files[], int af, proj_t *project, err_t *errfile, opt_t *options, int flags);
int rebuild_needed(file_t files[], int af, proj_t *project);
int build_it(proj_t *project, file_t files[], int af, err_t *errfile, opt_t *options);
int build(file_t files[], int af, proj_t *project, err_t *errfile, opt_t *options);
int clean(file_t files[], int af, proj_t *project, err_t *errfile, opt_t *options);
int install(file_t files[], int af, proj_t *project, err_t *errfile, opt_t *options);
void get_build_cmd(file_t *file, proj_t *project, char *cmd, char **outfile);
void set_makefile(proj_t *project, char *makefile, file_t *file, opt_t *options);
void init_makefile(proj_t *project, char *makefile, file_t *file, opt_t *options);
void set_project_run_cmd(proj_t *project, file_t *file);
int syntax_check(file_t files[], int af, opt_t *options, err_t *errfile);
int preprocess(file_t files[], int af, opt_t *options);
int spawn_build_cmd(char *argv[], char *outfile, err_t *errfile, proj_t *project, char *executable);
void profile(proj_t *project, file_t files[], int af, opt_t *options);
int errors(int status, err_t *errfile);
void skip_space(char **p);
void skip_nonspace(char **p);
char *resolve_make_var(proj_t *project, char *var, char **endp);
char *parse_value(proj_t *project, char *p);
void add_make_var(proj_t *project, char *ident, char *p);
void makefile_get_ident(char *line, char *ident, char **endp);
void parse_makefile(proj_t *project, char *makefile, opt_t *options);
int compiled_language(lang_t *lang);
/* build_opts.c */
int check_first_ext(lang_t *old_lang, lang_t *new_lang);
int language_options(file_t *file, opt_t *options);
void check_compiler_options(lang_t *lang, lang_t *old);
lang_t *check_build_opts(file_t *file);
int read_lang(char *lang_file, lang_t *lang);
int read_language_opts(lang_t **head);
int save_language(lang_t *lang);
void save_language_opts(lang_t *head);
int multispec_match(char *name_spec, char *str, int flags);
lang_t *get_bop(file_t *file, lang_t *head);
lang_t *new_bop(lang_t **head, char *filename);
lang_t *add_language(lang_t *head, char *name, char *name_spec, char *id_comment, char *compiler, char *compile_only_flag, char *compile_flags, char *link_flags, char *syntax_check_flag, char *debugger, char *debugger_flags, char *run_prefix, char *error_msg_format, char *executable_name, char *compile_output_flag, char *executable_spec, int auto_wrap, char *compile_to_asm_flag, char *preprocess_only_flag, char *upload_prefix, char *debugger_backtrace_cmd);
void destroy_bop_list(lang_t **head);
void init_lang(lang_t *lang);
/* cursor.c */
void scroll_file(file_t *file, opt_t *options, buff_t *cut_buff, direc_t direction);
void move_up(file_t *file, opt_t *options, buff_t *cut_buff);
void move_down(file_t *file, opt_t *options, buff_t *cut_buff);
void move_to_left_of_tab(file_t *file);
void move_left(file_t *file, opt_t *options, buff_t *cut_buff);
void end_line_no_redraw(file_t *file);
void end_line(file_t *file, opt_t *options, buff_t *cut_buff);
void move_right(file_t *file, opt_t *options, buff_t *cut_buff);
void move_ret(file_t *file, opt_t *options, buff_t *cut_buff);
void move_unret(file_t *file, opt_t *options, buff_t *cut_buff);
void home_file_no_redraw(file_t *file, opt_t *options, buff_t *cut_buff);
void home_file(file_t *file, opt_t *options, buff_t *cut_buff);
void end_file_no_redraw(file_t *file);
void end_file(file_t *file, opt_t *options, buff_t *cut_buff);
void begin_line(file_t *file, opt_t *options, buff_t *cut_buff);
void page_up(file_t *file, opt_t *options, buff_t *cut_buff);
void page_down(file_t *file, opt_t *options, buff_t *cut_buff);
void next_word(file_t *file, opt_t *options, buff_t *cut_buff);
void last_word(file_t *file, opt_t *options, buff_t *cut_buff);
void page_top(file_t *file, opt_t *options, buff_t *cut_buff);
void back_tab(file_t *file, opt_t *options, buff_t *cut_buff);
void gotoline(file_t *file, opt_t *options, buff_t *cut_buff);
void move_to(file_t *file, opt_t *options, buff_t *cut_buff, long linenum, long col);
void update_cursor(file_t *file, opt_t *options, buff_t *cut_buff);
void update_cursor_no_snap(file_t *file, opt_t *options, buff_t *cut_buff);
void snap_to_tab(file_t *file);
int check_column(file_t *file, opt_t *options, buff_t *cut_buff);
int check_column_no_snap(file_t *file, opt_t *options, buff_t *cut_buff);
/* custom.c */
int custom_menu(file_t files[], int *af_ptr, opt_t *options, event_t *event);
int run_unix(file_t files[], int aw, opt_t *options);
int run_custom_item(file_t files[], int *af_ptr, opt_t *options, cust_t menu_items[], int item_count, int ch);
void add_item(cust_t menu_items[], int item_count, opt_t *options);
int edit_item(cust_t menu_items[], int item_count, cust_t *item, opt_t *options, int new_item);
int taken(cust_t menu_items[], int item_count, int key);
void write_items(cust_t items[], int skip, int item_count);
int read_custom(char *custom_text[], cust_t menu_items[]);
void save_item(cust_t *item);
void run_item(file_t files[], int aw, cust_t menu_items[], int item_count, int ch, opt_t *options);
void expand_command(file_t *file, char *expanded, char *command);
void suspend(opt_t *options);
/* edit_m.c */
int edit_menu(file_t files[], int *aw, buff_t *cut_buff, opt_t *options, event_t *event);
void begin_area(file_t *file, buff_t *cut_buff);
void end_area(file_t *file, opt_t *options, buff_t *cut_buff);
void pull_area(file_t *file, buff_t *cut_buff);
void arrange_ends(buff_t *cut_buff);
void cut_area(file_t *file, opt_t *options, buff_t *cut_buff);
void delete_lines(file_t *file, int first, int last);
void restore_buff(file_t *file, int new_lines);
int make_room(file_t *file, int start_line, int new_lines);
void split_curline(file_t *file, int new_lines);
void insert_lines(file_t *file, buff_t *cut_buff, int indent);
void insert_partial_line(file_t *file, buff_t *cut_buff, int line, int col, int indent);
void paste_area(file_t *file, opt_t *options, buff_t *cut_buff, int auto_indent);
void append_partial_first_line(file_t *file, buff_t *cut_buff, int indent);
void unmark_area(buff_t *cut_buff);
void cancel_area(file_t *file, buff_t *cut_buff, opt_t *options);
int area_started(buff_t *cut_buff);
int area_ended(buff_t *cut_buff);
void init_cut_buff(buff_t *cut_buff);
int further(buff_t *cut_buff, int new_line, int new_col);
void adjust_highlight(file_t *file, buff_t *cut_buff, opt_t *options);
void set_highlight(file_t *file, opt_t *options);
void unset_highlight(file_t *file, opt_t *options);
void rewrite(file_t *file, opt_t *options, buff_t *cut_buff, size_t line1, int col1, size_t line2, int col2);
void rearrange(size_t *line1, buff_len_t *col1, size_t *line2, buff_len_t *col2);
int get_area(file_t *file, buff_t *cut_buff, size_t *line1, buff_len_t *col1, size_t *line2, buff_len_t *col2);
int in_area(file_t *file, buff_t *cut_buff, size_t line, int col, size_t *line1, buff_len_t *col1, size_t *line2, buff_len_t *col2);
int in_marked_area(file_t *file, buff_t *cut_buff, size_t line, int col);
int cursor_in_area(file_t *file, buff_t *cut_buff);
/* editbuff.c */
void insert_char(file_t *file, int key, opt_t *options, buff_t *cut_buff);
void insert_expand_tabs(file_t *file, int key, opt_t *options, buff_t *cut_buff);
void insert_tab_fillers(file_t *file, char *curchar, int count);
void insert_real_tabs(file_t *file, int key, opt_t *options, buff_t *cut_buff);
void adjust_next_tab(file_t *file);
int expand_buff_if_needed(file_t *file, size_t line, unsigned newlen);
void insert_expanded_tab(file_t *file, opt_t *options, buff_t *cut_buff);
int del_under(file_t *file, opt_t *options, buff_t *cut_buff);
int del_under_real_tabs(file_t *file, opt_t *options, buff_t *cut_buff);
void del_tab(file_t *file, char *p);
int del_under_expand_tabs(file_t *file, opt_t *options, buff_t *cut_buff);
void combine_lines(file_t *file, opt_t *options, buff_t *cut_buff);
void del_left(file_t *file, opt_t *options, buff_t *cut_buff);
void del_left_real_tabs(file_t *file, opt_t *options, buff_t *cut_buff);
void del_left_expand_tabs(file_t *file, opt_t *options, buff_t *cut_buff);
void new_line(file_t *file, opt_t *options, buff_t *cut_buff, unsigned long time_diff);
int new_line_buff(file_t *file, size_t curline, opt_t *options, unsigned long time_diff);
void cut_line(file_t *file, opt_t *options, buff_t *cut_buff);
void del_word(file_t *file, opt_t *options, buff_t *cut_buff);
void cut_to_end(file_t *file, opt_t *options, buff_t *cut_buff);
void indent_area(file_t *file, opt_t *options, buff_t *cut_buff, int cols);
void unindent_area(file_t *file, opt_t *options, buff_t *cut_buff, int cols);
void auto_wrap(file_t *file, opt_t *options, buff_t *cut_buff);
void join_lines(file_t *file, opt_t *options, buff_t *cut_buff);
int get_next_word(file_t *file, size_t *line, char **pp, char **word);
int format_paragraph(file_t *file, opt_t *options, buff_t *cut_buff);
int tab_stops(file_t *file);
/* error.c */
void view_errors(char *file);
void next_error(file_t *files, int *aw, err_t *errfile, opt_t *options, buff_t *cut_buff);
void remove_linefeed(char *line);
int read_error_msg(err_t *errfile, char mesg[], char source_file[], char **text, file_t *file, opt_t *options);
void goto_error(file_t *file, int compiler_line, opt_t *options, buff_t *cut_buff);
int display_error(file_t *file, size_t line, char *msg, opt_t *options);
void err_close(err_t *errfile);
long get_current_line(file_t *file, size_t compiler_line);
/* file_wins.c */
void update_win(file_t *file, opt_t *options, buff_t *cut_buff);
void update_line(file_t *file, opt_t *options, buff_t *cut_buff, size_t line, int start_col);
void check_highlight(file_t *file, opt_t *options, size_t line, int col, size_t line1, int col1, size_t line2, int col2);
void set_color(file_t *file, int no_color, int fg, int bg);
void edit_border(file_t *file, opt_t *options);
void toggle_file(file_t file[], int *af_ptr, opt_t *options, buff_t *cut_buff);
void select_compiler(file_t *file, opt_t *options);
void set_exe(file_t *file);
int blank_opts(lang_t *opt);
int get_free_win(file_t file[], char *dir_name, char *file_name, opt_t *options);
int open_in_aw(file_t files[], char *filename, char *dirname, opt_t *options);
void merge_file(file_t *file, char *path_name, opt_t *options, buff_t *cut_buff);
int more_lines(file_t *file);
int ape_putc(win_t *text, int ch);
/* files.c */
int file_menu(file_t files[], int *af_ptr, opt_t *options, buff_t *cut_buff, event_t *event);
int load_new_file(int ch, file_t files[], opt_t *options, unsigned int flags);
int save_as(file_t files[], int af, opt_t *options);
void view_header(file_t *file, opt_t *options);
int prompt_save_all(file_t files[], opt_t *options);
int prompt_save(file_t files[], int af, opt_t *options);
int open_file(file_t files[], char *path_name, opt_t *options, unsigned int flags);
int new_file(file_t *file);
int init_file(file_t *file, opt_t *options);
void create_edit_win(file_t *file, opt_t *options);
void set_colors(file_t *file, opt_t *options);
int get_dirname(char full_name[], char dir_name[], char base_name[]);
void close_file(file_t files[], int af, opt_t *options, int prompt);
int load_file(file_t *file, FILE *fp, opt_t *options);
int read_line(char string[], FILE *fp, file_t *file, opt_t *options);
int save_file(file_t *file, opt_t *options);
int write_line(FILE *fp, file_t *file, size_t l, opt_t *options);
void make_exe(char *file);
void backup_file(char *file);
int file_type(char *filename);
void new_blank_file(file_t files[], int *af_ptr, opt_t *options);
int file_undo(file_t *file, opt_t *options, buff_t *cut_buff);
int file_save_for_undo(file_t *file, undo_action_t undo_action, char *deleted_text);
/* help_m.c */
int help_menu(file_t *file, opt_t *options, event_t *event);
void man(char *str, char *prefix);
void get_word_at_cursor(file_t *file, char *word);
void apropos(char *topic);
void browse(char *file, opt_t *options);
/* init.c */
void load_keymap(term_t *terminal);
int do_args(int argc, char *argv[], file_t files[], opt_t *options);
int check_makefile(proj_t *project, char *makefile, file_t *file, opt_t *options);
void check_hostname(void);
void check_args(char *argv[]);
int init_xterm(void);
void register_signal_handlers(void);
void setup_terminal(void);
void init_compiler_lines(file_t files[], opt_t *options);
/* init_opts.c */
void add_c_cpp_patterns(pattern_t *patterns[]);
void add_c_patterns(pattern_t *patterns[]);
void add_cpp_patterns(pattern_t *patterns[]);
void add_csh_patterns(pattern_t *patterns[]);
void add_sh_patterns(pattern_t *patterns[]);
void add_fortran_patterns(pattern_t *patterns[]);
void init_options(opt_t *options);
void white_scheme(opt_t *options);
void blue_scheme(opt_t *options);
void black_scheme(opt_t *options);
void cyan_scheme(opt_t *options);
int install_default_user_config(opt_t *options);
/* macros.c */
int macro_menu(file_t files[], int *af_ptr, opt_t *options, event_t *event, macro_expand_t expand);
int macro_get_key(file_t *file, opt_t *options, event_t *event, char selected_text[], int line, int col);
int macro_valid_key(int ch, char *menu_text[]);
int macro_new_item(file_t files[], int *af_ptr, opt_t *options, buff_t *cut_buff, char *default_text);
int macro_remove(file_t *file, opt_t *options, event_t *event, char selected_text[]);
int macro_remove_body(char *path_name, int remove_ch, opt_t *options);
int read_macros(file_t *file, char *menu_text[]);
int macro_read_markup(FILE *fp, char *string, size_t max);
int macro_expand_markup(file_t *file, FILE *infp, FILE *outfp);
int macro_invoke(file_t *file, int ch, opt_t *options, macro_expand_t expand);
int macro_get_config_dir(file_t *file, char macro_dir[], size_t maxlen);
int macro_get_filename(file_t *file, char macro_filename[], size_t maxlen);
int macro_key(char *menu_text);
int macro_key_taken(file_t *file, int key);
int macro_read_header(FILE *infile, char *macro_key, int *flags, buff_t *macro_buff);
int macro_write_header(FILE *outfile, int macro_key, int flags, buff_t *macro_buff);
int menu_text_cmp(char **pp1, char **pp2);
int macro_replace(file_t files[], int *af_ptr, opt_t *options, buff_t *cut_buff, event_t *event);
int macro_edit(file_t files[], int *af_ptr, opt_t *options, buff_t *cut_buff, event_t *event);
int macro_new_submenu(file_t files[], int *af_ptr, opt_t *options, buff_t *cut_buff, char *default_text);
/* match.c */
void match_group(file_t *file, opt_t *options, buff_t *cut_buff);
void set_group_standout(file_t *file, opt_t *options);
void unmatch_group(file_t *file, opt_t *options, buff_t *cut_buff);
void highlight_char(file_t *file, char *p, size_t line, opt_t *options, buff_t *cut_buff);
/* messages.c */
void invalid_key(int key, char key_seq[]);
void print_seq(win_t *win, char *key_seq);
void display_mode(file_t *file);
void show_cursor_pos(opt_t *options, file_t *file);
int popup_mesg(char *format, char *buttons[], opt_t *options, ...);
/* mouse.c */
int col_to_menu(int col, term_t *term);
int button1_press(event_t *event, opt_t *options);
size_t get_release_line(file_t *file, event_t *event);
/* options.c */
int options_menu(file_t file[], int af, opt_t *options, buff_t *cut_buff, event_t *event);
void redraw_globals(opt_t *options, int old_no_color);
void redraw_screen(opt_t *options, file_t *file, buff_t *cut_buff);
int load_options(char *filename, opt_t *options);
void save_options(char *filename, opt_t *options);
int create_lang_options_if_missing(file_t *file, opt_t *options);
int debug_options(file_t *file, opt_t *options);
void draw_color_bar(win_t *win, int line, opt_t *options);
int screen_options(opt_t *options);
int misc_options(opt_t *options);
void get_borders(opt_t *options, bord_t *borders);
void set_borders(opt_t *options, bord_t *borders);
char *get_config_dir(char *dir, size_t maxlen);
int get_language_parent_dir(char language_parent_dir[], size_t maxlen);
int get_language_dir(lang_t *lang, char language_dir[], size_t maxlen);
/* reg.c */
void _reg(opt_t *options);
/* resize.c */
void win_resize(void);
void setup_resize(file_t file[], int *aw, opt_t *options, buff_t *cut_buff);
/* search_m.c */
int search_menu(file_t *file, char *string, opt_t *options, buff_t *cut_buff, event_t *event);
void display_case_sens(win_t *search_pop, opt_t *options);
void display_search_direction(win_t *search_pop, opt_t *options);
void find_string(file_t *file, char *string, opt_t *options, buff_t *cut_buff);
void replace_string(file_t *file, char *string, opt_t *options, buff_t *cut_buff, int resume_old_search);
int move_search(file_t *file, char *string, opt_t *options, buff_t *cut_buff, int start_line, int start_col, int visible_lines);
int compare(char *cur, char *search_str, opt_t *options);
int search_forward(file_t *file, char *string, opt_t *options, int start_line, int start_col, int visible_lines, buff_t *cut_buff);
int search_backward(file_t *file, char *string, opt_t *options, int start_line, int start_col, int visible_lines, buff_t *cut_buff);
char *get_x11_include(void);
void grep_headers(file_t *file, opt_t *options);
void search_libs(file_t *file, opt_t *options);
void set_popup_color(win_t *win, opt_t *options);
int set_search(char *search_string, buff_t *cut_buff, size_t str_max);
int wsmemcmp(char *buff, char *str);
int wsmemicmp(char *buff, char *str);
/* signal.c */
void kamakaze(void);
void notify(void);
void restore_help(void);
void set_globals_for_signal(file_t files[], opt_t *options);
/* subproc.c */
void begin_full_screen(void);
void end_full_screen(int pause);
void more(char *filename);
int run_command(int parent_action, int echo, char *cmd, char *shell);
int check_stat(int stat, char *cmd);
/* synhigh.c */
void synhigh_tag_which_lines(file_t *file, size_t first_guess, size_t last_guess, size_t *first_line, size_t *last_line);
int synhigh_tag_lines(file_t *file, size_t first_line, size_t last_line);
void synhigh_check_pattern_color(file_t *file, size_t line, char *ptr, char **end_pattern, opt_t *options, int *in_pattern);
int synhigh_check_patterns(file_t *file, size_t line, char *ptr, reloc_t *end);
void synhigh_set_pattern_color(file_t *file, opt_t *options, pattern_t *pattern);
void synhigh_clear_tags(file_t *file);
void synhigh_update(file_t *file, size_t line, opt_t *options, buff_t *cut_buff);
void check_language(file_t *file, opt_t *options, buff_t *cut_buff);
int synhigh_load_opts(char *filename, lang_t *lang);
int synhigh_add_pattern(pattern_t *patterns[], char *re, int fg, int bg, int modes);
int synhigh_save_opts(char *lang_dir, lang_t *lang);
int synhigh_compile_pattern(pattern_t *pattern);
void draw_modes_bar(win_t *win, int line, int col);
int synhigh_options(file_t *file, opt_t *options, buff_t *cut_buff);
void synhigh_free_patterns(pattern_t *patterns[]);
void update_lines(file_t *file, opt_t *options, buff_t *cut_buff, size_t first, size_t last);
/* undo_item.c */
int undo_item_set(undo_item_t *item, size_t line, size_t column, size_t length, undo_action_t action, const char *deleted_text);
/* undo_stack.c */
int undo_stack_push(undo_t *record, size_t line, size_t col, undo_action_t action, char *deleted_text);
undo_item_t *undo_stack_pop(undo_t *record);
void undo_stack_init(undo_t *record);
/* wins.c */
void sprintw(int start_col, int field_len, char *format, ...);
void stat_mesg(char *mesg);
void global_wins(opt_t *options);
void draw_menu_bar(opt_t *options);
void draw_status_bar(opt_t *options);
win_t *panel_win(int rows, int cols, int start_row, int start_col, opt_t *options);
win_t *centered_panel_win(int rows, int cols, opt_t *options);
int panel_get_string(file_t *file, opt_t *options, size_t len, const char *prompt, const char *help, tw_str_t string_type, char *string);

