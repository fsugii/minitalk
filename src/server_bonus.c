/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server_bonus.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fshiniti <fshiniti@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/22 20:00:34 by fshiniti          #+#    #+#             */
/*   Updated: 2025/12/02 21:37:12 by fshiniti         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minitalk.h"

static void	handle_signal(int sig_num, siginfo_t *sig_details, void *cpu_state)
{
	static int				bits;
	static unsigned char	c;

	(void)cpu_state;
	if (sig_num == SIGUSR2)
		c |= (1 << bits);
	bits++;
	if (bits == 8)
	{
		if (c == '\0')
			write (1, "\n", 1);
		else
			write (1, &c, 1);
		bits = 0;
		c = 0;
	}
	kill (sig_details->si_pid, SIGUSR1);
}

int	main(void)
{
	pid_t				pid;
	struct sigaction	action_struct;

	pid = getpid();
	ft_printf("Server PID: %d\n", pid);
	action_struct.sa_sigaction = handle_signal;
	sigemptyset(&action_struct.sa_mask);
	action_struct.sa_flags = SA_SIGINFO;
	sigaction (SIGUSR1, &action_struct, NULL);
	sigaction (SIGUSR2, &action_struct, NULL);
	while (1)
		pause();
}
