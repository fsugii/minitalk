/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fshiniti <fshiniti@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/22 20:00:34 by fshiniti          #+#    #+#             */
/*   Updated: 2025/11/30 18:53:10 by fshiniti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "minitalk.h"

static void	handle_signal(int sig)
{
	static int				bits;
	static unsigned char	c;

	if (sig == SIGUSR2)
		c |= (1 << bits);
	bits++;
	if (bits == 8)
	{
		write (1, &c, 1);
		bits = 0;
		c = 0;
	}
}

int	main(void)
{
	pid_t	pid;

	pid = getpid();
	ft_printf("Server PID: %d\n", pid);
	signal (SIGUSR1, handle_signal);
	signal (SIGUSR2, handle_signal);
	while (1)
		pause();
}
