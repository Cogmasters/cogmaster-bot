#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <concord/discord.h>
#include <concord/cog-utils.h>
#include <concord/log.h>

struct discord_guild *
get_guild(struct discord *cogbot)
{
    struct discord_guild *guild = calloc(1, sizeof *guild);
    char *path[] = { "cog_bot", "guild_id" };
    struct ccord_szbuf_readonly guild_id;
    CCORDcode code;

    guild_id = discord_config_get_field(cogbot, path, 2);
    assert(guild_id.size != 0 && "Missing cog_bot.guild_id");

    code = discord_get_guild(cogbot, strtoull(guild_id.start, NULL, 10),
                             &(struct discord_ret_guild){
                                 .sync = guild,
                             });

    if (code != CCORD_OK) {
        log_fatal("%s", discord_strerror(code, cogbot));
        exit(EXIT_FAILURE);
    }

    return guild;
}

struct discord_create_message *
get_components(const char fname[])
{
    struct discord_create_message *params = calloc(1, sizeof *params);
    size_t fsize = 0;
    char *fcontents;

    fcontents = cog_load_whole_file(fname, &fsize);
    assert(fcontents != NULL && "Missing file");
    assert(fsize != 0 && "Empty file");

    discord_create_message_from_json(fcontents, fsize, params);

    return params;
}

int
main(int argc, char *argv[])
{
    struct discord_create_message *params;
    struct discord_guild *guild;
    struct discord *cogbot;
    CCORDcode code;

    assert(argc > 1 && "Expect: ./menu <path>/menu.json <?config_file>");

    cogbot = discord_config_init((argc > 2) ? argv[2] : "../config.json");
    assert(NULL != cogbot && "Couldn't initialize cogbot");

    guild = get_guild(cogbot);
    params = get_components(argv[1]);

    code = discord_create_message(cogbot, guild->rules_channel_id, params,
                                  &(struct discord_ret_message){
                                      .sync = DISCORD_SYNC_FLAG,
                                  });

    if (code != CCORD_OK) {
        log_fatal("%s", discord_strerror(code, cogbot));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
