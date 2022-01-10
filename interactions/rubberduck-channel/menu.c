#include <stdlib.h>
#include <string.h>
#include <inttypes.h> /* PRIu64 */

#include <concord/discord.h>

#include "interactions.h"

/** @brief Per-request context storage for async functions */
struct context {
  /** the user that triggered the interaction */
  u64_snowflake_t user_id;
  /** the username for channel creation */
  char username[256];
  /** the client's application id */
  u64_snowflake_t application_id;
  /** the interaction token */
  char token[256];
  /** whether the channel is private or public */
  bool priv;
};

static void
done_create_channel(struct discord *cogbot, struct discord_async_ret *ret)
{
  const struct discord_channel *channel = ret->ret;
  struct context *cxt = ret->data;

  char welcome_msg[DISCORD_MAX_MESSAGE_LEN];

  snprintf(welcome_msg, sizeof(welcome_msg),
           "Welcome <@!%" PRIu64 ">, "
           "I hope you enjoy your new space! Check your available commands by "
           "typing `/mychannel` for further channel customization!",
           cxt->user_id);
  discord_async_next(cogbot, NULL);
  discord_create_message(cogbot, channel->id,
                         &(struct discord_create_message_params){
                           .content = welcome_msg,
                         },
                         NULL);

  discord_async_next(cogbot, NULL);
  discord_edit_original_interaction_response(
    cogbot, cxt->application_id, cxt->token,
    &(struct discord_edit_original_interaction_response_params){
      .content = "Your channel has been created!",
    },
    NULL);
}

static void
fail_create_channel(struct discord *cogbot, struct discord_async_err *err)
{
  struct context *cxt = err->data;

  discord_async_next(cogbot, NULL);
  discord_edit_original_interaction_response(
    cogbot, cxt->application_id, cxt->token,
    &(struct discord_edit_original_interaction_response_params){
      .content = "Couldn't create channel, please report to our staff.",
    },
    NULL);
}

static void
done_role_add(struct discord *cogbot, struct discord_async_ret *ret)
{
  struct cogbot_primitives *primitives = discord_get_data(cogbot);
  struct context *cxt = ret->data;

  /* create user channel */
  discord_async_next(cogbot, &(struct discord_async_attr){
                               .done = done_create_channel,
                               .fail = fail_create_channel,
                               .data = cxt,
                               .cleanup = &free,
                             });
  discord_create_guild_channel(
    cogbot, primitives->guild_id,
    &(struct discord_create_guild_channel_params){
      .name = cxt->username,
      .permission_overwrites =
        (struct discord_overwrite *[]){
          /* give read/write permission for user */
          &(struct discord_overwrite){
            .id = cxt->user_id,
            .type = 1,
            .allow = PERMS_READ | PERMS_WRITE,
          },
          /* give read/write permission for @helper */
          &(struct discord_overwrite){
            .id = primitives->roles.helper_id,
            .type = 0,
            .allow = PERMS_READ | PERMS_WRITE,
          },
          /* hide it from @watcher only if 'priv' has been set */
          &(struct discord_overwrite){
            .id = primitives->roles.watcher_id,
            .type = 0,
            .allow = cxt->priv ? 0 : PERMS_READ,
          },
          /* give write permissions to @everyone (not read) */
          &(struct discord_overwrite){
            .id = primitives->guild_id,
            .type = 0,
            .deny = PERMS_READ,
            .allow = PERMS_WRITE,
          },
          NULL, /* END OF OVERWRITE LIST */
        },
      .parent_id = primitives->category_id,
    },
    NULL);
}

static void
fail_role_add(struct discord *cogbot, struct discord_async_err *err)
{
  struct cogbot_primitives *primitives = discord_get_data(cogbot);
  struct context *cxt = err->data;

  char diagnosis[256];

  snprintf(diagnosis, sizeof(diagnosis),
           "Couldn't assign role <@&%" PRIu64 ">, please report to our staff.",
           primitives->roles.rubberduck_id);

  discord_async_next(cogbot, NULL);
  discord_edit_original_interaction_response(
    cogbot, cxt->application_id, cxt->token,
    &(struct discord_edit_original_interaction_response_params){
      .content = diagnosis,
    },
    NULL);

  free(cxt);
}

void
react_rubberduck_channel_menu(struct discord *cogbot,
                              struct discord_interaction_response *params,
                              const struct discord_interaction *interaction)
{
  struct cogbot_primitives *primitives = discord_get_data(cogbot);

  struct discord_guild_member *member = interaction->member;
  bool priv = false;

  /* skip user with already assigned channel */
  if (is_included_role(member->roles, primitives->roles.rubberduck_id)) {
    params->data->content =
      "It seems you already have a channel, please edit it from within";
    return;
  }

  /* get channel visibility */
  if (interaction->data->values)
    for (int i = 0; interaction->data->values[i]; ++i) {
      char *value = interaction->data->values[i]->value;

      if (0 == strcmp(value, "private")) priv = true;
    }

  struct context *cxt = malloc(sizeof *cxt);
  *cxt = (struct context){
    .user_id = member->user->id,
    .application_id = interaction->application_id,
    .priv = priv,
  };
  snprintf(cxt->username, sizeof(cxt->username), "%s", member->user->username);
  snprintf(cxt->token, sizeof(cxt->token), "%s", interaction->token);

  /* assign channel owner role to user */
  discord_async_next(cogbot, &(struct discord_async_attr){
                               .done = &done_role_add,
                               .fail = &fail_role_add,
                               .data = cxt,
                             });
  discord_add_guild_member_role(cogbot, primitives->guild_id, member->user->id,
                                primitives->roles.rubberduck_id);

  params->type =
    DISCORD_INTERACTION_CALLBACK_DEFERRED_CHANNEL_MESSAGE_WITH_SOURCE;
}
