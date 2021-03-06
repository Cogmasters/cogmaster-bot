#include <stdlib.h>
#include <string.h>
#include <inttypes.h> /* PRIu64 */

#include <concord/discord.h>
#include <concord/log.h>

#include "interactions.h"

/** @brief Per-request context storage for async functions */
struct context {
    /** the user to be muted or unmuted */
    u64snowflake target_id;
    /**
     * permissions to be denied from user
     *  - PERMS_WRITE: User will have his write access to channel revoked
     *  - 0: User won't be denied write access
     */
    u64bitmask perms;
};

static void
context_cleanup(struct discord *cogbot, void *p_cxt)
{
    (void)cogbot;
    free(p_cxt);
}

static void
done_edit_permissions(struct discord *cogbot, struct discord_response *resp)
{
    const struct discord_interaction *interaction = resp->keep;
    struct context *cxt = resp->data;
    char diagnosis[256];

    snprintf(diagnosis, sizeof(diagnosis),
             "Completed action targeted to <@!%" PRIu64 ">.", cxt->target_id);

    discord_edit_original_interaction_response(
        cogbot, interaction->application_id, interaction->token,
        &(struct discord_edit_original_interaction_response){
            .content = diagnosis,
        },
        NULL);
}

static void
fail_edit_permissions(struct discord *cogbot, struct discord_response *resp)
{
    const struct discord_interaction *interaction = resp->keep;
    struct context *cxt = resp->data;
    char diagnosis[256];

    snprintf(diagnosis, sizeof(diagnosis),
             "Failed action targeted to <@!%" PRIu64 ">", cxt->target_id);

    discord_edit_original_interaction_response(
        cogbot, interaction->application_id, interaction->token,
        &(struct discord_edit_original_interaction_response){
            .content = diagnosis,
        },
        NULL);
}

static void
done_get_channel(struct discord *cogbot,
                 struct discord_response *resp,
                 const struct discord_channel *channel)
{
    struct cogbot_primitives *primitives = discord_get_data(cogbot);
    const struct discord_interaction *interaction = resp->keep;

    if (!is_user_rubberduck_channel(channel, primitives->category_id,
                                    interaction->member->user->id))
    {
        discord_edit_original_interaction_response(
            cogbot, interaction->application_id, interaction->token,
            &(struct discord_edit_original_interaction_response){
                .content = "Couldn't complete operation. Make sure to use "
                           "`/mycommand` from your channel",
            },
            NULL);
    }
    else {
        struct context *cxt = resp->data;

        discord_edit_channel_permissions(
            cogbot, channel->id, cxt->target_id,
            &(struct discord_edit_channel_permissions){
                .type = 1,
                .deny = cxt->perms,
            },
            &(struct discord_ret){
                .done = &done_edit_permissions,
                .fail = &fail_edit_permissions,
                .keep = interaction,
                .data = cxt,
                .cleanup = &context_cleanup,
            });
    }
}

static u64snowflake
get_unmute_target(
    struct discord_guild_member *member,
    struct discord_application_command_interaction_data_options *options)
{
    u64snowflake target_id = 0ULL;

    if (options)
        for (int i = 0; i < options->size; ++i) {
            char *name = options->array[i].name;
            char *value = options->array[i].value;

            if (0 == strcmp(name, "user"))
                target_id = strtoull(value, NULL, 10);
        }

    /* TODO: post to a logging channel */
    log_info("Attempt to unmute user(%" PRIu64 ") by %s#%s", target_id,
             member->user->username, member->user->discriminator);

    return target_id;
}

static u64snowflake
get_mute_target(
    struct discord_guild_member *member,
    struct discord_application_command_interaction_data_options *options)
{
    u64snowflake target_id = 0ULL;
    char *reason = NULL;

    if (options)
        for (int i = 0; i < options->size; ++i) {
            char *name = options->array[i].name;
            char *value = options->array[i].value;

            if (0 == strcmp(name, "user"))
                target_id = strtoull(value, NULL, 10);
            else if (0 == strcmp(name, "reason"))
                reason = value;
        }

    /* TODO: post to a logging channel */
    log_info("Attempt to mute user(%" PRIu64 ") by %s#%s (%s)", target_id,
             member->user->username, member->user->discriminator, reason);

    return target_id;
}

void
react_rubberduck_channel_action(
    struct discord *cogbot,
    struct discord_interaction_response *params,
    const struct discord_interaction *interaction,
    struct discord_application_command_interaction_data_options *options)
{
    u64snowflake target_id = 0ULL;
    u64bitmask perms;

    if (options)
        for (int i = 0; i < options->size; ++i) {
            char *name = options->array[i].name;

            if (0 == strcmp(name, "mute")) {
                target_id = get_mute_target(interaction->member,
                                            options->array[i].options);
                perms = PERMS_WRITE;
            }
            else if (0 == strcmp(name, "unmute")) {
                target_id = get_unmute_target(interaction->member,
                                              options->array[i].options);
                perms = 0;
            }
        }

    if (target_id == interaction->member->user->id) {
        params->data->content = "You can't mute yourself!";
        return;
    }

    struct context *cxt = malloc(sizeof *cxt);
    *cxt = (struct context){
        .target_id = target_id,
        .perms = perms,
    };

    discord_get_channel(cogbot, interaction->channel_id,
                        &(struct discord_ret_channel){
                            .done = &done_get_channel,
                            .fail = &fail_edit_permissions,
                            .keep = interaction,
                            .data = cxt,
                            .cleanup = &context_cleanup,
                        });

    params->type = DISCORD_INTERACTION_DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE;
}
