#include <concord/discord.h>

#include "interactions.h"

bool
is_user_rubberduck_channel(const struct discord_channel *channel,
                           u64snowflake rubberduck_category_id,
                           u64snowflake user_id)
{
    if (channel->permission_overwrites)
        for (int i = 0; i < channel->permission_overwrites->size; ++i)
            if (user_id == channel->permission_overwrites->array[i].id
                && rubberduck_category_id == channel->parent_id)
            {
                return true;
            }

    return false;
}

bool
is_included_role(struct snowflakes *roles, u64snowflake role_id)
{
    for (int i = 0; i < roles->size; ++i)
        if (roles->array[i] == role_id) return true;
    return false;
}
