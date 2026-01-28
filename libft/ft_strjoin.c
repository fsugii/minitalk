/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strjoin.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fshiniti <fshiniti@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/17 23:29:53 by fshiniti          #+#    #+#             */
/*   Updated: 2025/04/18 00:07:32 by fshiniti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strjoin(char const *s1, char const *s2)
{
	unsigned int	i;
	unsigned int	j;
	char			*final;

	final = malloc(sizeof(char) * (ft_strlen(s1) + ft_strlen(s2) + 1));
	if (!final)
		return (NULL);
	if (!s1 || !s2)
		return (NULL);
	i = 0;
	while (s1[i])
	{
		final[i] = s1[i];
		i++;
	}
	j = 0;
	while (s2[j])
	{
		final[i] = s2[j];
		j++;
		i++;
	}
	final[i] = '\0';
	return (final);
}
/*
int main(void)
{
	char *dest = "ABC";
	char *src = "DEF";
	char *result = ft_strjoin(dest, src);
	printf ("Result: %s\n", result);
}*/
