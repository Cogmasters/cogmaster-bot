#include <stdlib.h>
#include <string.h>
#include <inttypes.h> /* PRIu64 */

#include <concord/discord.h>

#include "interactions.h"

/** @brief Per-request context storage for async functions */
struct context {
    /** the user that triggered the interaction */
    u64_snowflake_t user_id;
    /** the client's application id */
    u64_snowflake_t application_id;
    /** the interaction token */
    char token[256];
};

static void
done_delete_channel(struct discord *cogbot,
                    void *data,
                    const struct discord_channel *channel)
{
    struct cogbot_primitives *primitives = discord_get_data(cogbot);
    struct context *cxt = data;
    (void)channel;

    /* remove rubberduck role from user */
    discord_remove_guild_member_role(cogbot, primitives->guild_id,
                                     cxt->user_id,
                                     primitives->roles.rubberduck_id, NULL);
}

static void
fail_delete_channel(struct discord *cogbot, CCORDcode code, void *data)
{
    struct context *cxt = data;
    (void)code;

    discord_edit_original_interaction_response(
        cogbot, cxt->application_id, cxt->token,
        &(struct discord_edit_original_interaction_response){
            .content = "Couldn't delete channel, please contact our staff.",
        },
        NULL);
}

static void
done_get_channel(struct discord *cogbot,
                 void *data,
                 const struct discord_channel *channel)
{
    struct cogbot_primitives *primitives = discord_get_data(cogbot);
    struct context *cxt = data;

    if (!is_user_rubberduck_channel(channel, primitives->category_id,
                                    cxt->user_id))
    {
        discord_edit_original_interaction_response(
            cogbot, cxt->application_id, cxt->token,
            &(struct discord_edit_original_interaction_response){
                .content = "Couldn't complete operation. Make sure to use "
                           "`/mycommand` from your channel",
            },
            NULL);
    }
    else {
        discord_delete_channel(cogbot, channel->id,
                               &(struct discord_ret_channel){
                                   .done = &done_delete_channel,
                                   .fail = &fail_delete_channel,
                                   .data = cxt,
                               });
    }
}

void
react_rubberduck_channel_delete(
    struct discord *cogbot,
    struct discord_interaction_response *params,
    const struct discord_interaction *interaction,
    struct discord_application_command_interaction_data_option **options)
{
    struct discord_guild_member *member = interaction->member;
    bool confirm_action = false;
    char *reason = NULL;

    if (options)
        for (int i = 0; options[i]; ++i) {
            char *name = options[i]->name;
            char *value = options[i]->value;

            if (0 == strcmp(name, "confirm"))
                confirm_action = (0 == strcmp(value, "yes"));
            else if (0 == strcmp(name, "reason"))
                reason = value;
        }

    if (!confirm_action) {
        params->data->content = "No operation will be taking place.";
        return;
    }

    /* TODO: post to a logging channel */
    log_info("User '%s#%s' rubberduck channel deletion (%s)",
             member->user->username, member->user->discriminator, reason);

    struct context *cxt = malloc(sizeof *cxt);
    *cxt = (struct context){
        .user_id = member->user->id,
        .application_id = interaction->application_id,
    };
    snprintf(cxt->token, sizeof(cxt->token), "%s", interaction->token);

    discord_get_channel(cogbot, interaction->channel_id,
                        &(struct discord_ret_channel){
                            .done = &done_get_channel,
                            .fail = &fail_delete_channel,
                            .data = cxt,
                            .cleanup = &free,
                        });

    params->type =
        DISCORD_INTERACTION_CALLBACK_DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE;
}
